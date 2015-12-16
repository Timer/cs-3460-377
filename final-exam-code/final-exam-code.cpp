#define OVERRIDE_MATRIX_MULT true

#include <thread>
#include <future>
#include "../lib/matrix.hpp"
#include "../lib/pool.hpp"
#include "../lib/image.hpp"
#include "../lib/promise-polyfill.hpp"
#include "../lib/queue.hpp"

/* 1a */
matrix operator*(const matrix &x, const matrix &y) {
  if (x.cols != y.rows) {
    throw std::invalid_argument("Invalid arguments");
  }

  matrix z;
  z.create(x.rows, y.cols);

  //Flatten the loops so we can fairly chunk up the work (assume the
  //picture isn't the same width as it is height!)
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

/* 1b */
matrix histogram(const matrix &x) {
  matrix h{256, 1};
  parallel_for(0u, x.rows * x.cols, [&](int k) {
    unsigned i = k / x.cols, j = k % x.cols;
    auto value = x(i, j);
    if (value < 0) {
      h(0, 0)++;
    } else if (value >= 255) {
      h(255, 0)++;
    } else {
      h(value, 0)++;
    }
  });
  return h;
}

/*
 * 2. parallel_for uses the queue_work function instead of create_thread.
 * a. Why?
 * - Parallel for loops are typically used in data problems which require the
 * - computation of some value over a loop. As such, these are often short lived
 * - and cause blocks in control flow. To mitigate the delay and increase the
 * - benefit of using parallel_for, the work is pushed into the pool so that thread
 * - creation is not experienced (which takes time), additionally, since the 
 * - computation is most likely computationally expensive, the pool will only
 * - schedule the work when threads are available instead of maxing out the system
 * - and causing system lag.
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
  f.wait();//Wait for the async work to complete so our value is consistent every time
  return value;
}

/* 4 completed */
void conv(matrix &x, const matrix &k) {
  matrix y;
  y.create(x.rows + k.rows, x.cols + k.cols);

  const unsigned xR = x.rows, xC = x.cols;
  parallel_for(0u, xR * xC, [&](auto i) {
    auto row = i % xR, col = i / xR;
    auto yrow = row + k.rows / 2;
    auto ycol = col + k.cols / 2;
    y(yrow, ycol) = x(row, col);
  });

  std::atomic<int> weight(0);
  const unsigned kR = k.rows, kC = k.cols;
  parallel_for(0u, kR * kC, [&](int i) {
    auto row = i % kR, col = i / kR;
    weight += k(row, col);
  });

  parallel_for(0u, xR * xC, [&](int i) {
    auto row = i % xR, col = i / xR;
    int t = 0;
    auto yrow = row;
    for (int krow = k.rows - 1; krow >= 0; krow--, yrow++) {
      auto ycol = col;
      for (int kcol = k.cols - 1; kcol >= 0; kcol--, ycol++) {
        t += y(yrow, ycol) * k(krow, kcol);
      }
    }
    if (weight != 0) {
      t /= weight;
    }
    x(row, col) = t;
  });
}

int binomial_coefficient(int n, int k) {
  if (n <= 1 || k == 0) {
    return 1;
  } else {
    return binomial_coefficient(n - 1, k - 1) * n / k;
  }
}

matrix binomial(int n) {
  if ((n & 1) == 0) {
    throw std::invalid_argument("n must be odd");
  }

  matrix x, y;
  x.create(1, n);
  y.create(n, 1);

  for (int i = 0; i < n / 2; i++) {
    x(0, i) = x(0, n - i - 1) = binomial_coefficient(n - 1, i);
    y(i, 0) = y(n - i - 1, 0) = binomial_coefficient(n - 1, i);
  }

  x(0, n / 2) = binomial_coefficient(n - 1, n / 2);
  y(n / 2, 0) = binomial_coefficient(n - 1, n / 2);

  return y * x;
}

concurrent_queue<matrix> pipeline;
std::future<matrix> blur_image_async(matrix x) {
  return std::async(std::launch::async, [](matrix bmp) {
    matrix kernel = binomial(3);
    conv(bmp, kernel);
    return std::move(bmp);
  }, std::move(x));
}

int main_q4() {
  auto pipeline_thread = std::thread([] {
    for (;;) {
      auto x = pipeline.pop();
      auto b = blur_image_async(std::move(x)).share();
      Promise::then(b, [](auto f) {
        matrix h = histogram(f.get());
      }).wait();
    }
  });
  Promise::then(load_image_async("image.png").share(), [](auto f) {
    pipeline.push(f.get());
  }).wait();
  pipeline_thread.join();
  return 0;
}
/* 4 end */

/*
* 5a. How does a semaphore differ from mutex?
* - A semaphore differs from a mutex in that it is not a mutual exclusion lock,
* - a semaphore, unlike a mutex, can be acquired multiple times (depending on how
* - the semaphore is created) unlike a mutex which is strictly locked or unlocked.
* - Additionally, a semaphore lives in the file system and is not restricted to the
* - process in which it was created (like a mutex), which makes it useful to lock
* - access to things like shared memory pools across processes.
*
* 5b. Describe the operation of a bar (sitting down, ordering a drink) in terms
* of a semaphore and mutex.
* = Semaphore:
* - A bar has `n` number of seats, so a semaphore is constructed accordingly.
* - The semaphore (or a bar seat) can be acquired up to `n` times before parties
* - begin waiting for a seat at the bar. Meaning, each person can reserve a seat
* - by sitting (acquiring) it and then leaving (releasing) the seat, where then
* - the fastest person to react can acquire the released seat. This experiences
* - contention when the semaphore is fully acquired, but not at the same level as
* - a mutex.
*
* = Mutex:
* - The bar has a single seat and the bar tender refuses to serve anyone who is
* - not sitting at that seat. The seat can be locked (sat in) and then unlocked
* - (individual leaves). When the seat is unlocked, the fastest person to react
* - can then sit (lock) in the seat. This means that after the seat is taken,
* - every new person who wishes to have service must wait, which creates contention
* - in the form of a large blob of people (many threads waiting in the kernel on
* - the lock).
*/

int main(int argc, char **argv) {
  return 0;
}
