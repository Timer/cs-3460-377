#include "../lib/shared_mem.hpp"
#include "shared.hpp"
#include <cstdio>

#define PROCESS_COUNT 3
#define COMPUTE_TO 9009
#define BLOCK_COUNT (COMPUTE_TO / 1000) + 1

std::shared_ptr<cs477::bounded_queue<primes_request_1000, BLOCK_COUNT>> rq_queue;
std::shared_ptr<cs477::bounded_queue<primes_response_1000, BLOCK_COUNT>> rp_queue;

int main(int argc, char **argv) {
	rq_queue = std::make_shared<cs477::bounded_queue<primes_request_1000, BLOCK_COUNT>>();
	rq_queue->create("primes-rq-queue");
	rp_queue = std::make_shared<cs477::bounded_queue<primes_response_1000, BLOCK_COUNT>>();
	rp_queue->create("primes-rp-queue");

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

	for (auto i = 0; i < requests; ++i) {
		auto r = rp_queue->read();
	}
  return 0;
}
