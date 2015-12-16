#include <cstdint>

struct primes_request_1000 {
  uint32_t start;
};

struct primes_response_1000 {
  uint32_t count;
  uint32_t primes[1000];
};
