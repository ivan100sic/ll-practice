#include <atomic>
#include <iostream>
#include <map>
#include <thread>

namespace demos {

namespace orders {

struct Weak {
  inline static constexpr auto Load = std::memory_order_relaxed;
  inline static constexpr auto Store = std::memory_order_relaxed;
};

struct Intermediate {
  inline static constexpr auto Load = std::memory_order_acquire;
  inline static constexpr auto Store = std::memory_order_release;
};

struct Strong {
  inline static constexpr auto Load = std::memory_order_seq_cst;
  inline static constexpr auto Store = std::memory_order_seq_cst;
};

} // namespace orders

const int MaxSteps = 10'000'000;

namespace relaxed {

struct Context {
  std::atomic_bool go_flag_1;
  std::atomic_bool go_flag_2;
  std::atomic_bool abort_flag;
  std::atomic_bool t1_done;
  std::atomic_bool t2_done;

  char separation_0[4096];

  std::atomic_int x;

  char separation_1[4096];

  std::atomic_int y;

  char separation_2[4096];

  std::atomic_int r1;

  char separation_3[4096];

  std::atomic_int r2;
};

template <class Strength> void thread1(Context *ctx) {
  while (!ctx->abort_flag) {
    while (!ctx->go_flag_1.exchange(false))
      ;
    int val = ctx->y.load(Strength::Load);
    ctx->x.store(val, Strength::Store);
    ctx->r1 = val;
    ctx->t1_done = true;
  }
}

template <class Strength> void thread2(Context *ctx) {
  while (!ctx->abort_flag) {
    while (!ctx->go_flag_2.exchange(false))
      ;
    int val = ctx->x.load(Strength::Load);
    ctx->y.store(42, Strength::Store);
    ctx->r2 = val;
    ctx->t2_done = true;
  }
}

template <class Strength> void driver() {
  Context ctx;
  ctx.go_flag_1 = false;
  ctx.go_flag_2 = false;
  ctx.abort_flag = false;
  ctx.t1_done = false;
  ctx.t2_done = false;

  int steps = MaxSteps;

  std::thread t1(thread1<Strength>, &ctx);
  std::thread t2(thread2<Strength>, &ctx);

  int weird_orderings = 0;
  for (int step = 0; step < MaxSteps; step++) {
    ctx.x = 0;
    ctx.y = 0;
    if (step == MaxSteps - 1) {
      ctx.abort_flag = true;
    }
    ctx.t1_done = false;
    ctx.t2_done = false;
    ctx.go_flag_1 = true;
    ctx.go_flag_2 = true;

    while (!(ctx.t1_done && ctx.t2_done))
      ;

    if (ctx.r1 == 42 && ctx.r2 == 42) {
      weird_orderings++;
    }
  }

  std::cerr << "weird_orderings: " << weird_orderings << '\n';

  t1.join();
  t2.join();
}

/**
 * Unfortunately this demo is inconclusive.
 * I was unable to reproduce the example shown here:
 * https://en.cppreference.com/w/cpp/atomic/memory_order.html#:~:text=Explanation-,Relaxed%20ordering,-Atomic%20operations%20tagged
 *
 * Probably because x86 has strong memory ordering.
 */
void main() {
  driver<orders::Weak>();
  driver<orders::Intermediate>();
  driver<orders::Strong>();
}

} // namespace relaxed

namespace store_reordering {

static constexpr size_t Separation = 4096;

struct Context {
  std::atomic_bool go_flag_1;
  std::atomic_bool go_flag_2;
  std::atomic_bool abort_flag;
  std::atomic_bool t1_done;
  std::atomic_bool t2_done;

  char separation_0[Separation];

  std::atomic_int x1;

  char separation_1[Separation];

  std::atomic_int y1;

  char separation_2[Separation];

  std::atomic_int x2;

  char separation_3[Separation];

  std::atomic_int y2;
};

template <class Strength> void thread1(Context *ctx) {
  while (!ctx->abort_flag) {
    while (!ctx->go_flag_1.exchange(false))
      ;
    ctx->x1.store(1, Strength::Store);
    ctx->y1.store(2, Strength::Store);
    ctx->t1_done = true;
  }
}

template <class Strength, bool ReverseLoading> void thread2(Context *ctx) {
  while (!ctx->abort_flag) {
    while (!ctx->go_flag_2.exchange(false))
      ;
    int x, y;
    if constexpr (ReverseLoading) {
      y = ctx->y1.load(Strength::Load);
      x = ctx->x1.load(Strength::Load);
    } else {
      x = ctx->x1.load(Strength::Load);
      y = ctx->y1.load(Strength::Load);
    }
    ctx->x2.store(x, Strength::Store);
    ctx->y2.store(y, Strength::Store);
    ctx->t2_done = true;
  }
}

template <class Strength, bool ReverseLoading> void driver() {
  Context ctx;
  ctx.go_flag_1 = false;
  ctx.go_flag_2 = false;
  ctx.abort_flag = false;
  ctx.t1_done = false;
  ctx.t2_done = false;

  int steps = MaxSteps;

  std::thread t1(thread1<Strength>, &ctx);
  std::thread t2(thread2<Strength, ReverseLoading>, &ctx);

  std::map<std::pair<int, int>, int> counts;
  for (int step = 0; step < MaxSteps; step++) {
    ctx.x1 = 0;
    ctx.y1 = 0;
    if (step == MaxSteps - 1) {
      ctx.abort_flag = true;
    }
    ctx.t1_done = false;
    ctx.t2_done = false;
    ctx.go_flag_1 = true;
    ctx.go_flag_2 = true;

    while (!(ctx.t1_done && ctx.t2_done))
      ;

    counts[{ctx.x2.load(), ctx.y2.load()}]++;
  }

  std::cerr << typeid(Strength).name()
            << (ReverseLoading ? " reverse\n" : " same\n");
  std::cerr << "counts:\n";
  for (auto [key, val] : counts) {
    std::cerr << key.first << ' ' << key.second << ": " << val << '\n';
  }

  t1.join();
  t2.join();
}

/**
 * It looks like store reorderings for different variables
 * can happen regardless of the chosen memory ordering.
 *
 * Silly me. This is absolutely expected. If the two threads
 * execute the instructions in the order
 *   L1, S1, S2, L2
 * then this happens.
 *
 * However, if we load the variables x, y in reverse order, the (0, 2) case
 * becomes impossible. It should be possible on some architectures, though.
 */
void main() {
  driver<orders::Weak, false>();
  driver<orders::Intermediate, false>();
  driver<orders::Strong, false>();

  driver<orders::Weak, true>();
  driver<orders::Intermediate, true>();
  driver<orders::Strong, true>();
}

} // namespace store_reordering

namespace intel_example {

static constexpr size_t Separation = 4096;

struct Context {
  std::atomic_bool go_flag_1;
  std::atomic_bool go_flag_2;
  std::atomic_bool abort_flag;
  std::atomic_bool t1_done;
  std::atomic_bool t2_done;

  char separation_0[Separation];

  std::atomic_int slot1;

  char separation_1[Separation];

  std::atomic_int slot2;

  char separation_2[Separation];

  std::atomic_int result1;

  char separation_3[Separation];

  std::atomic_int result2;
};

template <class Strength> void thread1(Context *ctx) {
  while (!ctx->abort_flag) {
    while (!ctx->go_flag_1.exchange(false))
      ;
    ctx->slot1.store(1, Strength::Store);
    int val = ctx->slot2.load(Strength::Load);
    ctx->result1.store(val, Strength::Store);
    ctx->t1_done = true;
  }
}

template <class Strength> void thread2(Context *ctx) {
  while (!ctx->abort_flag) {
    while (!ctx->go_flag_2.exchange(false))
      ;
    ctx->slot2.store(1, Strength::Store);
    int val = ctx->slot1.load(Strength::Load);
    ctx->result2.store(val, Strength::Store);
    ctx->t2_done = true;
  }
}

template <class Strength> void driver() {
  Context ctx;
  ctx.go_flag_1 = false;
  ctx.go_flag_2 = false;
  ctx.abort_flag = false;
  ctx.t1_done = false;
  ctx.t2_done = false;

  int steps = MaxSteps;

  std::thread t1(thread1<Strength>, &ctx);
  std::thread t2(thread2<Strength>, &ctx);

  std::map<std::pair<int, int>, int> counts;
  for (int step = 0; step < MaxSteps; step++) {
    ctx.slot1 = 0;
    ctx.slot2 = 0;
    if (step == MaxSteps - 1) {
      ctx.abort_flag = true;
    }
    ctx.t1_done = false;
    ctx.t2_done = false;
    ctx.go_flag_1 = true;
    ctx.go_flag_2 = true;

    while (!(ctx.t1_done && ctx.t2_done))
      ;

    counts[{ctx.result1.load(), ctx.result2.load()}]++;
  }

  std::cerr << typeid(Strength).name() << '\n';
  std::cerr << "counts:\n";
  for (auto [key, val] : counts) {
    std::cerr << key.first << ' ' << key.second << ": " << val << '\n';
  }

  t1.join();
  t2.join();
}

/**
 * At last, a conclusive demo.
 * In the seq-cst ordering, the (0, 0) case is impossible.
 */
void main() {
  driver<orders::Weak>();
  driver<orders::Intermediate>();
  driver<orders::Strong>();
}

} // namespace intel_example

} // namespace demos

int main(int argc, char **argv) { demos::intel_example::main(); }
