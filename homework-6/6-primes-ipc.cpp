#include "../lib/shared_mem.hpp"
#include "../homework-6/shared.hpp"
#include "../lib/process.hpp"
#include "../lib/promise-polyfill.hpp"
#include <cstdio>

std::shared_ptr<cs477::bounded_queue<primes_request_1000, BLOCK_COUNT>> rq_queue;
std::shared_ptr<cs477::bounded_queue<primes_response_1000, BLOCK_COUNT>> rp_queue;

cs477::process wp;
void create_worker_process();

int main(int argc, char **argv) {
  rq_queue = std::make_shared<cs477::bounded_queue<primes_request_1000, BLOCK_COUNT>>();
  rq_queue->create("primes-rq-queue");
  rp_queue = std::make_shared<cs477::bounded_queue<primes_response_1000, BLOCK_COUNT>>();
  rp_queue->create("primes-rp-queue");

  create_worker_process();

  puts("[DRIVER] Writing requests ...");

  auto requests = 0;
  auto curr = 0;
  for (;;) {
    primes_request_1000 r;
    r.start = curr;
    curr = min(curr + 1000, COMPUTE_TO);
    r.end = curr;
    rq_queue->write(r);
    ++requests;
    if (curr == COMPUTE_TO) break;
  }

  puts("[DRIVER] Listening for responses ...");

  std::vector<uint32_t> primes;

  for (auto i = 0; i < requests; ++i) {
    puts("[DRIVER] Requesting a response ...");
    auto r = rp_queue->read();
    puts("[DRIVER] Got a response ... let's utilize it !");
    for (int i = 0;; ++i) {
      if (r.primes[i] == 0) break;
      primes.push_back(r.primes[i]);
    }
  }
  printf("Distributedly, we found %d primes between %d and %d ...\n", primes.size(), 0, COMPUTE_TO);
  return 0;
}

void create_worker_process() {
  std::async(std::launch::async, [] {
    puts("[DRIVER] Spawning worker ...");
    auto o = cs477::create_process("..\\x64\\Debug\\homework-6-p2.exe", "");
  });
}