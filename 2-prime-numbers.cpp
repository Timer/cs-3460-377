#include "lib/pool.hpp"
#include <cstdio>
#include <cstdlib>

inline bool is_prime(const long& n) {
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

std::vector<long> get_primes(long start, long end) {
  std::vector<long> v;
  if (start <= 2 && end > 2) {
    v.push_back(2);
  }
  if (start % 2 == 0) ++start;
  for (long n = start; n < end; n += 2) {
    if (is_prime(n)) v.push_back(n);
  }
  return v;
}

int main(int argc, char** argv) {
  long compute_to = 1000000;
  if (argc == 2) compute_to = atoi(argv[1]);

  Pool pool;
  const int POOL_SIZE = pool.size();
  printf("Using a pool size of %d to compute primes under %d.\n", POOL_SIZE, compute_to);

  const long PER_BLOCK = compute_to / POOL_SIZE;
  const long ADDL = compute_to - PER_BLOCK * POOL_SIZE;
  long c = 0;
  std::vector<std::future<std::vector<long>>> vs;
  for (int i = 0; i < POOL_SIZE; ++i) {
    long start = c;
    c += PER_BLOCK + (i < ADDL ? 1 : 0);
    long end = c;
    printf("Thread %d from %d to %d.\n", i, start, end);
    vs.push_back(pool.submit(get_primes, start, end));
  }
  std::vector<long> primes;
  for (auto v = vs.begin(); v != vs.end(); ++v) {
    std::vector<long> sub = v->get();
    primes.insert(std::end(primes), std::begin(sub), std::end(sub));
  }
  printf("There are %d primes under %d.\n", primes.size(), compute_to);
  return 0;
}
