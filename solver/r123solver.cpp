#include "cracker.h"
#include "cracker_var.h"

#define R1_MULTI_OFFSET 0
#define R2_MULTI_OFFSET MULTIBITS(R1LEN)
#define R3_MULTI_OFFSET (R2_MULTI_OFFSET + MULTIBITS(R2LEN))
#define R1_OFFSET (R3_MULTI_OFFSET + MULTIBITS(R3LEN))
#define R2_OFFSET (R1_OFFSET + R1LEN)
#define R3_OFFSET (R2_OFFSET + R2LEN)

typedef Matrix<A5SRC_LEN + 1, SOLUTION_FRAMES * KGY> SolvMatrix;

struct Solution {
	u_int R1_value;
	u_int R2_value;
	u_int R3_value;
	u_int R1_mask;
	u_int R2_mask;
	u_int R3_mask;
};

static int
is_R1_single(u_int pos) {
	return pos >= R1_OFFSET && pos < R1_OFFSET + R1LEN;
}

static int
is_R2_single(u_int pos) {
	return pos >= R2_OFFSET && pos < R2_OFFSET + R2LEN;
}

static int
is_R3_single(u_int pos) {
	return pos >= R3_OFFSET && pos < R3_OFFSET + R3LEN;
}

static int
is_single(u_int pos) {
	return is_R1_single(pos) || is_R2_single(pos) || is_R3_single(pos);
}

static int
is_R1_multi(u_int pos) {
	return (pos + 1) >= (R1_MULTI_OFFSET + 1)
		&& pos < R1_MULTI_OFFSET + MULTIBITS(R1LEN);
}

static int
is_R2_multi(u_int pos) {
	return pos >= R2_MULTI_OFFSET
		&& pos < R2_MULTI_OFFSET + MULTIBITS(R2LEN);
}

static int
is_R3_multi(u_int pos) {
	return (pos + 1) >= (R3_MULTI_OFFSET + 1)
		&& pos < R3_MULTI_OFFSET + MULTIBITS(R3LEN);
}

/* if bit is singlevar and == 1:
 *   reset it in all rows;
 *   flip result value in all rows where bit N is set;
 *   for each multivalue with this bit reset multivar
 *     and flip other singlevar bit
 */
/* if bit is singlevar and == 0:
 *   reset it in all rows;
 *   reset all multivars with it in all rows.
 */
static void
process_single_var(SolvMatrix &matrix, u_int pos, u_int value, Solution &s) {
	u_int bitnum = 999;
	u_int multioffset = 0;
	u_int regoffset = 0;
	u_int reglen = 0;
	if (is_R1_single(pos)) {
		bitnum = pos - R1_OFFSET;
		multioffset = R1_MULTI_OFFSET;
		regoffset = R1_OFFSET;
		reglen = R1LEN;
		s.R1_mask |= 1 << (R1LEN - 1 - bitnum);
		if (value) {
			s.R1_value |= 1 << (R1LEN - 1 - bitnum);
		}
	} else if (is_R2_single(pos)) {
		bitnum = pos - R2_OFFSET;
		multioffset = R2_MULTI_OFFSET;
		regoffset = R2_OFFSET;
		reglen = R2LEN;
		s.R2_mask |= 1 << (R2LEN - 1 - bitnum);
		if (value) {
			s.R2_value |= 1 << (R2LEN - 1 - bitnum);
		}
	} else if (is_R3_single(pos)) {
		bitnum = pos - R3_OFFSET;
		multioffset = R3_MULTI_OFFSET;
		regoffset = R3_OFFSET;
		reglen = R3LEN;
		s.R3_mask |= 1 << (R3LEN - 1 - bitnum);
		if (value) {
			s.R3_value |= 1 << (R3LEN - 1 - bitnum);
		}
	} else {
		assert(0);
	}
	assert(bitnum < reglen);
	for (u_int y = 0; y < matrix.ysize; y++) {
		for (u_int b = bitnum + 1; b < reglen; b++) {
			u_int offset = multioffset + b * (b - 1) / 2 + bitnum;
			if (value && matrix.test(offset, y)) {
				matrix.flip(regoffset + b, y);
			}
			matrix.reset(offset, y);
		}
		for (u_int b = 0; b < bitnum; b++) {
			u_int offset = multioffset
				+ bitnum * (bitnum - 1) / 2 + b;
			if (value && matrix.test(offset, y)) {
				matrix.flip(regoffset + b, y);
			}
			matrix.reset(offset, y);
		}
	}
}

/* if bit is multivar and == 1:
 *   reset this multivar in all rows;
 *   flip result bit in all rows where multivar was set;
 *   get 2 singlebits == 1 and work with them.
 */
/* if bit is multivar and == 0:
 *   reset this multivar in all rows.
 */

void
process_multi_var(SolvMatrix &matrix, u_int pos, u_int value, Solution &s) {
	if (value == 0) {
		/* nothing to do */
		return;
	}

	u_int bitnum = 999;
	u_int regoffset = 0;
	u_int reglen = 0;
	if (is_R1_multi(pos)) {
		bitnum = pos - R1_MULTI_OFFSET;
		regoffset = R1_OFFSET;
		reglen = R1LEN;
	} else if (is_R2_multi(pos)) {
		bitnum = pos - R2_MULTI_OFFSET;
		regoffset = R2_OFFSET;
		reglen = R2LEN;
	} else if (is_R3_multi(pos)) {
		bitnum = pos - R3_MULTI_OFFSET;
		regoffset = R3_OFFSET;
		reglen = R3LEN;
	} else {
		assert(0);
	}
	assert(bitnum < MULTIBITS(reglen));

	u_int varc = 0;
	for (u_int b1 = 0; b1 < reglen; b1++) {
		for (u_int b2 = b1 + 1; b2 < reglen; b2++) {
			u_int offset = b2 * (b2 - 1) / 2 + b1;
			if (offset == bitnum) {
				varc++;
				process_single_var(matrix, regoffset + b1,
						value, s);
				process_single_var(matrix, regoffset + b2,
						value, s);
			}
		}
	}
	assert(varc == 1);
}

static int
simplify_matrix(SolvMatrix &matrix, Solution &s) {
	int rv = 0;

	for (u_int y = 0; y < matrix.ysize; y++) {
		u_int f = 0;
		u_int pos = 0;
		for (u_int x = 0; x < A5SRC_LEN; x++) {
			if (matrix.test(x, y)) {
				f++;
				pos = x;
				if (f > 1) {
					break;
				}
			}
		}
		if (f != 1) {
			continue;
		}
		rv = 1;
		u_int value = matrix.test(A5SRC_LEN, y);

		for (u_int j = 0; j < matrix.ysize; j++) {
			/* for all cases: reset found var, adjust answer */
			if (value && matrix.test(pos, j)) {
				matrix.flip(A5SRC_LEN, j);
			}
			matrix.reset(pos, j);
		}
		if (is_single(pos)) {
			process_single_var(matrix, pos, value, s);
		} else {
			process_multi_var(matrix, pos, value, s);
		}
	}
	return rv;
}

void
find_a5_states(SolvMatrix &matrix, Solution &s) {
	s.R1_value = s.R2_value = s.R3_value = 0;
	s.R1_mask = s.R2_mask = s.R3_mask = 0;

	do {
		matrix.gauss(A5SRC_LEN);
	} while (simplify_matrix(matrix, s));
}

void
get_ini_values(const FrameInfo data[SOLUTION_FRAMES], u_int R4,
		u_int &R1, u_int &R2, u_int &R3) {
	assert(R4 & (1 << 10));

	if (! Kg_initialized) {
		init_Kg(Kg);
		Kg_initialized = 1;
	}

	SolvMatrix solver;

	for (u_int n = 0; n < SOLUTION_FRAMES; n++) {
		A5Matrix S;
		make_a5_matrix(S, R4, data[0].frame_number & 0x0f,
				data[n].frame_number & 0x0f, data[n].reverse);

		Matrix<A5SRC_LEN, KGY> mat2;
		mat2.make_product(S, Kg);

		Vec<KGY> kgout;
		Kg.multiply(data[n].data, kgout);

		for (u_int j = 0; j < KGY; j++) {
			for (u_int i = 0; i < A5SRC_LEN; i++) {
				if (mat2.test(i, j)) {
					solver.set(i, n * KGY + j);
				}
			}
			if (kgout.test(j)) {
				solver.set(A5SRC_LEN, n * KGY + j);
			}
		}
	}

	Solution s;
	find_a5_states(solver, s);

	assert(s.R1_mask == R1MASK);
	assert(s.R2_mask == R2MASK);
	assert(s.R3_mask == R3MASK);

	assert(s.R1_value & (1 << 15));
	assert(s.R2_value & (1 << 16));
	assert(s.R3_value & (1 << 18));

	R1 = s.R1_value;
	R2 = s.R2_value;
	R3 = s.R3_value;
}
