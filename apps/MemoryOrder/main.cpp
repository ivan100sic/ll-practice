#include <atomic>
#include <iostream>
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
 */
void main() {
  driver<orders::Weak>();
  driver<orders::Intermediate>();
  driver<orders::Strong>();
}

} // namespace relaxed

} // namespace demos

int main(int argc, char **argv) { demos::relaxed::main(); }
