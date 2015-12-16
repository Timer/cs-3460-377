#define OVERRIDE_MATRIX_MULT true

#include "../lib/matrix.hpp"
#include "../lib/pool.hpp"

/* 1a */
matrix operator*(const matrix &x, const matrix &y) {
  if (x.cols != y.rows) {
    throw std::invalid_argument("Invalid arguments");
  }

  matrix z;
  z.create(x.rows, y.cols);

  parallel_for(0u, x.rows * y.cols, [&](int k) {
    unsigned i = k / y.cols, j = k % y.cols;
    int zz = 0;
    for (unsigned k = 0; k < x.cols; k++) {
      zz += x(i, k) * y(k, j);
    }
    z(i, j) = zz;
  });
  return z;
}

/*
 * 2. parallel_for uses the queue_work function instead of create_thread.
 * a. Why?
 * - Parallel for loops are typically used in data problems which require the
 * - computation of some value over a loop. As such, these are often short lived
 * - and cause blocks in control flow. To mitigate the delay and increase the
 * - benefit of using parallel_for, the work is pushed into the pool so that thread
 * - creation is not experienced (which takes time), additionally, since the computation
 * - is most likely computationally expensive, the pool will only schedule the work
 * - when threads are available instead of maxing out the system and causing system lag.
 *
 * b. Give an example of when should you use create_thread instead of queue_work
 * to execute work asynchronously?
 * - Create thread should always be used for work which is *long running* or blocking.
 * - Reason being is that long running work (or threads which will be alive the entire
 * - program life) will utilize all the threads provisioned to the pool and will cease
 * - execution of other work (e.g. a worker thread that works on a concurrent queue).
 * - Knowing this, simply spawn a new thread!
 */

/* 3 */
//Consider the following program
int mainEx() {
  int value = 32;
  queue_work([&] {
    value -= 17;
    printf("%d\n", value);
  });
  return value;
}

/*
* 3a.  What is the result of this program?  (Hint: its not always the same).
* - The result of this program varies depending on how quickly the queued
* - works starts in relation to the execution of the remaining code (pop stack
* - [removing queue_work return], push `value`, and return stack[0]/int).
*
* Possible results:
* 1. mainEx returns 32
* 2. mainEx returns 15
* 3. mainEx returns 15 and prints 15
*/

/* 3b - Correct the defect. */
int mainExFixed() {
  int value = 32;
  auto f = queue_work([&] {
    value -= 17;
    printf("%d\n", value);
  });
  f.wait();
  return value;
}

/*
* 5a. How does a semaphore differ from mutex?
* - A semaphore differs from a mutex in that it is not a mutual exclusion lock,
* - a semaphore, unlike a mutex, can be aquired multiple times (depending on how
* - the sempahore is created) unlike a mutex which is strictly locked or unlocked.
* - Additionally, a semaphore lives in the file system and is not restrictied to the
* - process in which it was created (like a mutex), which makes it useful to lock
* - access to things like shared memory pools across processes.
*
* 5b. Describe the operation of a bar (sitting down, ordering a drink) in terms
* of a semaphore and mutex.
*/

int main(int argc, char **argv) {
  return 0;
}
