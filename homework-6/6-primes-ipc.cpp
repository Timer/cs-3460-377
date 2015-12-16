#include "../lib/shared_mem.hpp"
#include "shared.hpp"
#include <iostream>

const int PROCESS_COUNT = 3;

std::shared_ptr<cs477::bounded_queue<primes_request_1000, 4000>> rq_queue;
std::shared_ptr<cs477::bounded_queue<primes_response_1000, 4000>> rp_queue;

int main(int argc, char **argv) {
	rq_queue = std::make_shared<cs477::bounded_queue<primes_request_1000, 4000>>();
	rq_queue->create("primes-rq-queue");
	rp_queue = std::make_shared<cs477::bounded_queue<primes_response_1000, 4000>>();
	rp_queue->create("primes-rp-queue");

	for (auto i = 0; i < PROCESS_COUNT; ++i) {
	}

	std::cin.get();
  return 0;
}
