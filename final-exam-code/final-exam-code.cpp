#define OVERRIDE_MATRIX_MULT true

#include "../lib/matrix.hpp"
#include "../lib/pool.hpp"

/* 1a */
matrix operator *(const matrix &x, const matrix &y)
{
	if (x.cols != y.rows)
	{
		throw std::invalid_argument("Invalid arguments");
	}

	matrix z;
	z.create(x.rows, y.cols);

	parallel_for(0u, x.rows * y.cols, [&](int k) {
		unsigned i = k / y.cols, j = k % y.cols;
		int zz = 0;
		for (unsigned k = 0; k < x.cols; k++)
		{
			zz += x(i, k) * y(k, j);
		}
		z(i, j) = zz;
	});
	return z;
}

int main(int argc, char **argv) {
  return 0;
}
