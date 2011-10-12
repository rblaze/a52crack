#pragma once

#include "defs.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

template <u_int vecsize> class Vec {
public:
	/** Create zero vector */
	Vec(void) { clear(); }

	/** Create vector from string */
	explicit Vec(const char *str);

	/** Create vector from byte array */
	explicit Vec(const unsigned char *arr);

	/** Create vector with copy of data */
	template <u_int s> explicit Vec(const Vec<s> &src);

	/** Fill vector with random values */
	void fill_random(void) {
		clear();
		for (u_int i = 0; i < size; i++) {
			if (wordparity(::random())) {
				this->set(i);
			}
		}
	}

	/** Clear vector */
	void clear(void) {
		::memset(data, 0, rowsize * sizeof(u_int32_t));
	}

	/** Set one bit */
	void set(u_int pos) {
		assert(pos < size);
		data[pos / WORDBITS] |= onebit(pos % WORDBITS);
	}

	/** Flip one bit */
	void flip(u_int pos) {
		assert(pos < size);
		data[pos / WORDBITS] ^= onebit(pos % WORDBITS);
	}
	
	/** Test bit value */
	int test(u_int pos) const { 
		assert(pos < size);
		return (data[pos / WORDBITS] & onebit(pos % WORDBITS)) ? 1:0; 
	}

	/** Test parity of vector */
	int vparity(void) const { 
		u_int32_t p = data[0];
		for (u_int i = 1; i < rowsize; i++) {
			p ^= data[i];
		}
		return ::wordparity(p);
	}

	/** XOR with another vector */
	void operator ^=(const Vec<vecsize> &other) {
		for (u_int i = 0; i < rowsize; i++) {
			this->data[i] ^= other.data[i];
		}
	}

	/** AND with another vector */
	void operator &=(const Vec<vecsize> &other) {
		for (u_int i = 0; i < rowsize; i++) {
			this->data[i] &= other.data[i];
		}
	}

	void print_bin(void) const;
	void print_hex(void) const;

	int operator==(const Vec<vecsize> &other) const {
		return ! ::memcmp(this->data, other.data, rowsize * sizeof(u_int32_t));
	}

	static const u_int size = vecsize;

protected:
	template <u_int> friend class Vec;
	template <u_int, u_int> friend class Matrix;

	static u_int32_t onebit(u_int b) {
		assert(b < WORDBITS);
		return htonl(1 << (WORDBITS - 1 - b));
	}

	static const u_int rowsize = howmany(vecsize, WORDBITS);
	u_int32_t data[rowsize];
};

template <u_int, u_int> class RotatedMatrix;

template <u_int x_size, u_int y_size> class Matrix {
public:
	/** Create empty matrix */
	Matrix(void) { ::memset(data, 0, rowsize * y_size * sizeof(u_int32_t)); }

	/** Fill matrix as product of two others */
	template <u_int step> void make_product(const Matrix<x_size, step> &first, const Matrix<step, y_size> &second);

	template <u_int step> void make_product(const RotatedMatrix<x_size, step> &first, const Matrix<step, y_size> &second);

	/** Multiply matrix to vector */
	void multiply(const Vec<x_size> &in, Vec<y_size> &out) const;

	/** Check if product of matrix and vector is zero */
	int zero_product(const Vec<x_size> &in) const;

	/** Set bit */
	void set(u_int x, u_int y) {
		assert(x < x_size);
		assert(y < y_size);
		data[y * rowsize + x / WORDBITS] |= onebit(x % WORDBITS);
	}

	/** Flip bit */
	void flip(u_int x, u_int y) {
		assert(x < x_size);
		assert(y < y_size);
		data[y * rowsize + x / WORDBITS] ^= onebit(x % WORDBITS);
	}

	/** Reset bit */
	void reset(u_int x, u_int y) {
		assert(x < x_size);
		assert(y < y_size);
		data[y * rowsize + x / WORDBITS] &= ~onebit(x % WORDBITS);
	}


	/** Test bit */
	int test(u_int x, u_int y) const {
		assert(x < x_size);
		assert(y < y_size);
		return (data[y * rowsize + x / WORDBITS]
				& onebit(x % WORDBITS)) ? 1:0;
	}

	/** Perform N Gauss elimination steps */
	u_int gauss(u_int steps);

	/** Solve first N matrix columns in terms of other columns */
	u_int solve(u_int steps);

	static const u_int xsize = x_size;
	static const u_int ysize = y_size;

protected:
	template <u_int, u_int> friend class Matrix;
	/* make row multiple of 4 ints in size to allow vector ops. */
	static const u_int rowsize = howmany(x_size, WORDBITS);

	static u_int32_t onebit(u_int b) {
		return Vec<x_size>::onebit(b % WORDBITS);
	}

	u_int32_t data[rowsize * ysize];
};

template <u_int x_size, u_int y_size> class RotatedMatrix
		: private Matrix<y_size, x_size> {
public:
	/** Create empty matrix */
	RotatedMatrix(void) : Matrix<y_size, x_size>() { /* no-op */ }

	/** Multiply matrix to vector */
	void multiply(const Vec<x_size> &in, Vec<y_size> &out) const;

	/** Set bit */
	void set(u_int x, u_int y) {
		Matrix<y_size, x_size>::set(y, x);
	}

	/** Flip bit */
	void flip(u_int x, u_int y) {
		Matrix<y_size, x_size>::flip(y, x);
	}

	/** Reset bit */
	void reset(u_int x, u_int y) {
		Matrix<y_size, x_size>::reset(y, x);
	}


	/** Test bit */
	int test(u_int x, u_int y) const {
		return Matrix<y_size, x_size>::test(y, x);
	}

	static const u_int xsize = x_size;
	static const u_int ysize = y_size;

	template <u_int, u_int> friend class Matrix;
};

template <u_int vecsize>
Vec<vecsize>::Vec(const char *str) {
	assert(::strlen(str) * 8 == vecsize);
	clear();
	::memcpy(data, str, vecsize / 8);
}

template <u_int vecsize>
Vec<vecsize>::Vec(const unsigned char *arr) {
	clear();
	::memcpy(data, arr, howmany(vecsize, 8));

	u_int lastbits = vecsize % WORDBITS;
	if (lastbits) {
		u_int32_t z = htonl(0xffffffff << (WORDBITS - lastbits));
		this->data[rowsize - 1] &= z;
	}
}

template <u_int vecsize>
template <u_int s>
Vec<vecsize>::Vec(const Vec<s> &src) {
	static_assert(vecsize >= s, "can't copy big vector to small");
	clear();
	::memcpy(this->data, src.data, src.rowsize * sizeof(u_int32_t));
}

template <u_int vecsize>
void
Vec<vecsize>::print_bin(void) const {
	for (u_int i = 0; i < size; i++) {
		printf("%d", this->test(i));
	}
	printf("\n");
}

template <u_int vecsize>
void
Vec<vecsize>::print_hex(void) const {
	const u_int8_t *p = data;
	for (u_int i = 0; i < size / 8; i++) {
		printf("%02hhx ", p[i]);
	}
	printf("\n");
}

template <u_int x_size, u_int y_size>
template <u_int step>
void
Matrix<x_size, y_size>::make_product(const Matrix<x_size, step> &first, 
					   const Matrix<step, y_size> &second) {
	::memset(data, 0, rowsize * y_size * sizeof(u_int32_t));

	for (u_int i = 0; i < y_size; i++) {
		for (u_int j = 0; j < x_size; j++) {
			bool v = false;
			for (u_int m = 0; m < step; m++) {
				if (first.test(j, m) && second.test(m, i)) {
					v = !v;
				}
			}
			if (v) {
				this->set(j, i);
			}
		}
	}
}

template <u_int x_size, u_int y_size>
template <u_int step>
void
Matrix<x_size, y_size>::make_product(const RotatedMatrix<x_size, step> &first, 
					   const Matrix<step, y_size> &second) {
	::memset(data, 0, rowsize * y_size * sizeof(u_int32_t));
	assert(first.rowsize == second.rowsize);

	/* Multiply matrices by word */
	for (u_int i = 0; i < y_size; i++) {
		for (u_int j = 0; j < x_size; j++) {
			u_int32_t v = 0;
			for (u_int m = 0; m < first.rowsize; m++) {
				u_int32_t b = first.data[j * first.rowsize + m]
					& second.data[i * first.rowsize + m];
				v ^= b;
			}
			if (wordparity(v)) {
				this->set(j, i);
			}
		}
	}
}

template <u_int x_size, u_int y_size>
void
Matrix<x_size, y_size>::multiply(const Vec<x_size> &in,
		Vec<y_size> &out) const {
	out.clear();

	for (u_int i = 0; i < y_size; i++) {
		u_int32_t p = 0;
		for (u_int j = 0; j < rowsize; j++) {
			p ^= in.data[j] & data[i * rowsize + j];
		}
		if (wordparity(p)) {
			out.set(i);
		}
	}
}

template <u_int x_size, u_int y_size>
int
Matrix<x_size, y_size>::zero_product(const Vec<x_size> &in) const {
	for (u_int i = 0; i < y_size; i++) {
		u_int32_t p = 0;
		for (u_int j = 0; j < rowsize; j++) {
			p ^= in.data[j] & data[i * rowsize + j];
		}
		if (wordparity(p)) {
			return 0;
		}
	}
	return 1;
}

template <u_int x_size, u_int y_size>
void
RotatedMatrix<x_size, y_size>::multiply(const Vec<x_size> &in,
		Vec<y_size> &out) const {
	out.clear();

	for (u_int i = 0; i < y_size; i++) {
		u_int32_t p = 0;
		for (u_int j = 0; j < x_size; j++) {
			p ^= in.test(j) & this->test(j, i);
		}
		if (p) {
			out.set(i);
		}
	}
}

template <u_int x_size, u_int y_size>
u_int
Matrix<x_size, y_size>::gauss(u_int steps) {
	assert(steps <= y_size);
	assert(steps <= x_size);

	u_int row = 0;

	for (u_int b = 0; b < steps; b++) {
		u_int y;

		for (y = row; y < y_size; y++) {
			if (test(b, y)) {
				break;
			}
		}
		if (y == y_size) {
			/* bit not found, continue */
			continue;
		}

		if (y != row) {
			/* move this row up */
			u_int32_t *dst = data + row * rowsize;
			u_int32_t *src = data + y * rowsize;
			for (u_int i = 0; i < rowsize; i++) {
				dst[i] ^= src[i];
			}
		}
		/* remove this bit from any other row */
		for (y = row + 1; y < y_size; y++) {
			if (! test(b, y)) {
				continue;
			}
			u_int32_t *src = data + row * rowsize;
			u_int32_t *dst = data + y * rowsize;
			for (u_int i = 0; i < rowsize; i++) {
				dst[i] ^= src[i];
			}
		}
		row++;
	}

#ifndef NDEBUG
#warning DEBUG MODE
	/* verify results */
	for (u_int y = 0; y < steps; y++) {
		for (u_int b = 0; b < y; b++) {
			assert(test(b, y) == 0);
		}
	}
	for (u_int y = steps; y < y_size; y++) {
		for (u_int b = 0; b < steps; b++) {
			assert(test(b, y) == 0);
		}
	}
#endif

	return row;
}

template <u_int x_size, u_int y_size>
u_int
Matrix<x_size, y_size>::solve(u_int steps) {
	gauss(steps);

	for (u_int b = 0; b < steps - 1; b++) {
		for (u_int y = 0; y < steps - 1 - b; y++) {
			if (test(steps - 1 - b, y) == 0) {
				continue;
			}

			/* move this row up */
			u_int32_t *dst = data + y * rowsize;
			u_int32_t *src = data + (steps - 1 - b) * rowsize;
			for (u_int i = 0; i < rowsize; i++) {
				dst[i] ^= src[i];
			}
		}
	}

#ifndef NDEBUG
	/* verify results */
	for (u_int y = 0; y < steps; y++) {
		for (u_int b = 0; b < steps; b++) {
			assert(test(b, y) == (b == y));
		}
	}
#endif

	return 0;
}
