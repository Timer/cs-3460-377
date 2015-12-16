#include "../lib/shared_mem.hpp"
#include <iostream>

const int PROCESS_COUNT = 3;

std::shared_ptr<cs477::shm_pool> pool;
std::shared_ptr<cs477::bounded_queue<cs477::message, 1024>> rq_queue;
std::shared_ptr<cs477::bounded_queue<cs477::message, 1024>> rp_queue;

int main(int argc, char **argv) {
	pool = std::make_shared<cs477::shm_pool>();
	pool->create("primes-pool", 4096, 16384);
	rq_queue = std::make_shared<cs477::bounded_queue<cs477::message, 1024>>();
	rq_queue->create("primes-rq-queue");
	rp_queue = std::make_shared<cs477::bounded_queue<cs477::message, 1024>>();
	rp_queue->create("primes-rp-queue");

	for (auto i = 0; i < PROCESS_COUNT; ++i) {
		cs477::buffer prime_block{pool};
		prime_block.allocate(4);
		rq_queue->write(prime_block.make_message());
	}

	std::cout << "test" << std::endl;
	std::cin.get();
  return 0;
}
