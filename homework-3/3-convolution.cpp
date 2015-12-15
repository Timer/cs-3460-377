#include "../lib/matrix.hpp"
#include "../lib/image.hpp"
#include "../lib/time.hpp"
#include "../lib/pool.hpp"
#include <atomic>

void conv(Pool &pool, matrix &x, const matrix &k) {
  matrix y;
  y.create(x.rows + k.rows, x.cols + k.cols);

  const unsigned xR = x.rows, xC = x.cols;
  pool.parallel_for(0u, xR * xC, [&](auto i) {
    auto row = i % xR, col = i / xR;
    auto yrow = row + k.rows / 2;
    auto ycol = col + k.cols / 2;
    y(yrow, ycol) = x(row, col);
  });

  std::atomic<int> weight(0);
  const unsigned kR = k.rows, kC = k.cols;
  pool.parallel_for(0u, kR * kC, [&](int i) {
    auto row = i % kR, col = i / kR;
    weight += k(row, col);
  });

  pool.parallel_for(0u, xR * xC, [&](int i) {
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

int main(int argc, char **argv) {
  auto bmp = load_image("C:\\Users\\Joe\\Desktop\\test.jpg");
  auto orig = bmp;

  matrix kernel = binomial(3);

  Pool pool;
  auto start = now();
  conv(pool, bmp, kernel);
  printf("Blurred in %d ms.\n", to_milliseconds(start, now()));

  save_png(bmp, "C:\\Users\\Joe\\Desktop\\test.png");
  return 0;
}
