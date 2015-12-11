#include "../lib/pool.hpp"
#include <cstdio>

int main(int argc, char **argv) {
  Pool pool(1);
  pool.submit([]() {
    puts("Hello world!");
  }).wait();
  return 0;
}
