#include <vector>
#include <cstdlib>
#include <time.h>
#include <algorithm>
#include <thread>
#include <cstdio>
#include <cmath>
#include "lib/time.hpp"
#include "lib/pool.hpp"

template <class Iter>
void merge_sort(std::atomic<int> &threads, Pool &pool, int forks, const Iter &start, const Iter &end) {
  if (start >= end) return;
  if (forks < 2) {
    std::sort(start, end);
  } else {
    Iter mid = start + (end - start) / 2;
    threads += 2;
    forks -= 2;
    auto job1 = pool.submit([&]() {
      merge_sort(threads, pool, forks / 2, start, mid);
    });
    auto job2 = pool.submit([&]() {
      merge_sort(threads, pool, forks / 2, mid, end);
    });
    job1.wait();
    job2.wait();
    std::inplace_merge(start, mid, end);
  }
}

template <class Iter>
void merge_sort(Pool &pool, const Iter &start, const Iter &end) {
  std::atomic<int> spawned(0);
  merge_sort(spawned, pool, pool.size(), start, end);
  printf("%d threads were actually spawned (this only plays nice when threads is 2^n).\n", spawned.load());
}

int main(int argc, char **argv) {
  Pool pool(std::thread::hardware_concurrency());
  srand(time(nullptr));
  int array_size = 100000000;
  puts("Filling ...");
  std::vector<int> v;
  v.reserve(array_size);
  for (int i = 0; i < array_size; ++i) v.push_back(rand());
  printf("Sorting %d values with %d threads ...\n", array_size, pool.size());
  auto t = now();
  merge_sort(pool, v.begin(), v.end());
  printf("Sorted in %d ms.\n", to_milliseconds(t, now()));
  for (int i = 1; i < array_size; ++i) {
    if (v[i - 1] > v[i]) {
      puts("Not sorted?");
      return 1;
    }
  }
  puts("Validated sort.");
  return 0;
}
