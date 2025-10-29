#include <practice/utils/timing.hpp>

#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>

namespace false_sharing {

const int Steps = 10'000'000;

constexpr uint8_t ArgumentReady = 1;
constexpr uint8_t ConsumerDone = 2;
constexpr uint8_t ProducerDone = 3;

struct Context {
  std::atomic_uint8_t flag = 0;
  std::atomic_uint64_t arg = 0;
};

std::mutex g_log_mtx;

template <class Func> void producer(Context *ctx, int id) {
  practice::utils::Timing timer;
  Func f;
  for (int i = 0; i < Steps; i++) {
    while (ctx->flag != ConsumerDone)
      ;
    ctx->arg = f();
    ctx->flag = ArgumentReady;
  }

  {
    std::unique_lock lck(g_log_mtx);
    std::cerr << "Producer " << id << " done in " << timer.get() << " s\n";
  }

  ctx->flag = ProducerDone;
}

void consumer(Context *ctx, int id) {
  uint64_t sum = 0;
  while (1) {
    while (ctx->flag == ConsumerDone)
      ;
    if (ctx->flag == ProducerDone) {
      break;
    }

    sum += ctx->arg;
    ctx->flag = ConsumerDone;
  }

  {
    std::unique_lock lck(g_log_mtx);
    std::cerr << "Consumer done: sum =  " << sum << '\n';
  }
}

template <size_t Gap> struct GappedQuadruple {
  Context c0;
  char g0[Gap];
  Context c1;
  char g1[Gap];
  Context c2;
  char g2[Gap];
  Context c3;
};

struct Integers {
  uint64_t next = 0;

  uint64_t operator()() { return ++next; }
};

struct Evens {
  uint64_t next = 0;

  uint64_t operator()() { return next += 2; }
};

struct Odds {
  uint64_t next = 1;

  uint64_t operator()() {
    auto res = next;
    next += 2;
    return res;
  }
};

struct Bits {
  uint64_t next = 1;

  uint64_t operator()() { return next ^= 1; }
};

template <size_t Gap> void driver() {
  GappedQuadruple<Gap> gq;

  gq.c0.flag = ConsumerDone;
  gq.c1.flag = ConsumerDone;
  gq.c2.flag = ConsumerDone;
  gq.c3.flag = ConsumerDone;

  std::thread p0(producer<Integers>, &gq.c0, 0);
  std::thread p1(producer<Evens>, &gq.c1, 1);
  std::thread p2(producer<Odds>, &gq.c2, 2);
  std::thread p3(producer<Bits>, &gq.c3, 3);

  std::thread c0(consumer, &gq.c0, 0);
  std::thread c1(consumer, &gq.c1, 1);
  std::thread c2(consumer, &gq.c2, 2);
  std::thread c3(consumer, &gq.c3, 3);

  p0.join();
  p1.join();
  p2.join();
  p3.join();
  c0.join();
  c1.join();
  c2.join();
  c3.join();
}

void main() {
  std::cerr << "Gap 1 B\n";
  driver<1>();
  std::cerr << "Gap 64 B\n";
  driver<64>();
}

} // namespace false_sharing

int main() { false_sharing::main(); }
