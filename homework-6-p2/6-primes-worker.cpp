#include "../lib/shared_mem.hpp"
#include "../homework-6/shared.hpp"
#include "../lib/pool.hpp"
#include <cstdio>

std::shared_ptr<cs477::bounded_queue<primes_request_1000, BLOCK_COUNT>> rq_queue;
std::shared_ptr<cs477::bounded_queue<primes_response_1000, BLOCK_COUNT>> rp_queue;

bool is_prime(const long& n) {
  if (n <= 3) {
    return n > 1;
  } else if (n % 2 == 0 || n % 3 == 0) {
    return false;
  } else {
    for (long i = 5; i * i <= n; i += 6) {
      if (n % i == 0 || n % (i + 2) == 0) return false;
    }
  }
  return true;
}

int main(int argc, char** argv) {
  rq_queue = std::make_shared<cs477::bounded_queue<primes_request_1000, BLOCK_COUNT>>();
  rq_queue->create("primes-rq-queue");
  rp_queue = std::make_shared<cs477::bounded_queue<primes_response_1000, BLOCK_COUNT>>();
  rp_queue->create("primes-rp-queue");

  for (;;) {
    puts("[WORKER] Requesting work ...");
    auto w = rq_queue->read();
    printf("[WORKER] Got work request ... computing primes between %d and %d.\n", w.start, w.end);
    primes_response_1000 r;
    std::atomic<int> count(0);
    parallel_for(w.start, w.end, [&](const uint32_t n) {
      if (is_prime(n)) r.primes[count++] = n;
    });
    r.primes[count.load()] = 0;
    printf("[WORKER] Found %d primes between %d and %d ...\n", count.load(), w.start, w.end);
    puts("[WORKER] Writing response ...");
    rp_queue->write(r);
    puts("[WORKER] Wrote response ...");
  }
  return 0;
}