#include <cstdint>

#define COMPUTE_TO 100000
#define BLOCK_COUNT (COMPUTE_TO / 1000) + 1

struct primes_request_1000 {
  uint32_t start;
  uint32_t end;
};

struct primes_response_1000 {
  uint32_t primes[1000];
};
