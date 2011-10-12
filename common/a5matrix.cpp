#include "a5.h"
#include "a5matrix.h"

#define FRAMESKIP 99
#define FRAMESIZE FACCH3_BLOCK_SIZE

#define min(a,b) (a < b ? a : b)
#define max(a,b) (a > b ? a : b)

typedef Vec<R1LEN> R1Vec;
typedef Vec<R2LEN> R2Vec;
typedef Vec<R3LEN> R3Vec;

const u_int R1ini = 0x0004f148;
const u_int R2ini = 0x003ff3ff;
const u_int R3ini = 0x00414ccd;
const u_int R4ini = 0x00001f38;

static void
clock_rows(u_int &r1, u_int &r2, u_int &r3) {
	int clock_R1, clock_R2, clock_R3;

	R4clock(clock_R1, clock_R2, clock_R3);

	if (clock_R1) {
		r1++;
	}
	if (clock_R2) {
		r2++;
	}
	if (clock_R3) {
		r3++;
	}
}

static void
fill_R1_states(R1Vec *states, u_int count, u_int bitmask) {
	// set first states
	for (u_int i = 0; i < R1LEN; i++) {
		states[i].set(i);
		if (i != 3) {
			const u_int LS = 2 * R1LEN - FRAMELEN;
			int f = (bitmask >> (i + FRAMELEN - R1LEN)) & 1;
			if (i > LS - 1) {
				f ^= (bitmask >> (i - LS)) & 1;
			}
			if (i > LS - 2) {
				f ^= (bitmask >> (i - LS + 1)) & 1;
			}
			if (i > LS - 3) {
				f ^= (bitmask >> (i - LS + 2)) & 1;
			}
			if (i > LS - 6) {
				f ^= (bitmask >> (i - LS + 5)) & 1;
			}
			if (f) {
				states[i].set(R1LEN - 1 - 15);
			}
		}
	}
	// fill other states
	for (u_int i = R1LEN; i < count; i++) {
		states[i] ^= states[i - 19];
		states[i] ^= states[i - 18];
		states[i] ^= states[i - 17];
		states[i] ^= states[i - 14];
	}
}

static void
fill_R2_states(R2Vec *states, u_int count, u_int bitmask) {
	// set first states
	for (u_int i = 0; i < R2LEN; i++) {
		states[i].set(i);
		if (i != 5) {
			int f = (bitmask >> i) & 1;
			if (i > 20) {
				f ^= (bitmask >> (i - 21)) & 1;
			}
			if (f) {
				states[i].set(R2LEN - 1 - 16);
			}
		}
	}
	// fill other states
	for (u_int i = R2LEN; i < count; i++) {
		states[i] ^= states[i - 22];
		states[i] ^= states[i - 21];
	}
}

static void
fill_R3_states(R3Vec *states, u_int count, u_int bitmask) {
	// set first states
	for (u_int i = 0; i < R3LEN; i++) {
		states[i].set(i);
		if (i >= R3LEN - FRAMELEN && i != 4) {
			int f = (bitmask >> (i - R3LEN + FRAMELEN)) & 1;
			if (i > 8) {
				f ^= (bitmask >> (i - 9)) & 1;
			}
			/* 2*8 */
			if (i > 16) {
				f ^= (bitmask >> (i - 17)) & 1;
			}
			if (i > 21) {
				f ^= (bitmask >> (i - 22)) & 1;
			}
			if (f) {
				states[i].set(R3LEN - 1 - 18);
			}
		}
	}
	// fill other states
	for (u_int i = R3LEN; i < count; i++) {
		states[i] ^= states[i - 23];
		states[i] ^= states[i - 22];
		states[i] ^= states[i - 21];
		states[i] ^= states[i - 8];
	}
}

static void
expand_a5_states(u_int R1, u_int R2, u_int R3, A5State &state) {
	state.clear();

	for (u_int i = 0; i < R1LEN; i++) {
		if ((R1 >> (R1LEN - 1 - i)) & 1) {
			state.set(R1_OFFSET + i);
			for (u_int j = i + 1; j < R1LEN; j++) {
				if ((R1 >> (R1LEN - 1 - j)) & 1) {
					state.set(R1_MULTI_OFFSET + j * (j - 1) / 2 + i);
				}
			}
		}
	}

	for (u_int i = 0; i < R2LEN; i++) {
		if ((R2 >> (R2LEN - 1 - i)) & 1) {
			state.set(R2_OFFSET + i);
			for (u_int j = i + 1; j < R2LEN; j++) {
				if ((R2 >> (R2LEN - 1 - j)) & 1) {
					state.set(R2_MULTI_OFFSET + j * (j - 1) / 2 + i);
				}
			}
		}
	}

	for (u_int i = 0; i < R3LEN; i++) {
		if ((R3 >> (R3LEN - 1 - i)) & 1) {
			state.set(R3_OFFSET + i);
			for (u_int j = i + 1; j < R3LEN; j++) {
				if ((R3 >> (R3LEN - 1 - j)) & 1) {
					state.set(R3_MULTI_OFFSET + j * (j - 1) / 2 + i);
				}
			}
		}
	}
}

template <u_int size>
static void
multiply_reg(const Vec<size> &w1, const Vec<size> &w2, A5Matrix &matrix,
		u_int row, u_int offset, u_int multioffset) {
	for (u_int bit1 = 0; bit1 < size; bit1++) {
		if (! w1.test(bit1)) {
			continue;
		}
		for (u_int bit2 = 0; bit2 < size; bit2++) {
			if (! w2.test(bit2)) {
				continue;
			}
			if (bit1 == bit2) {
				// flip single bit, a * a = a
				matrix.flip(offset + bit1, row);
			} else {
				u_int bmax = max(bit1, bit2);
				u_int bmin = min(bit1, bit2);
				u_int bitidx = bmax * (bmax - 1) / 2
					+ bmin;
				matrix.flip(multioffset + bitidx, row);
			}
		}
	}
}

static void
verify_a5_matrix(const A5Matrix &matrix, u_int first_frame, u_int base_shift,
		u_int R4base, int reverse) {
	A5State inistate;
	Vec<A5Matrix::ysize> answer;
	u_int R1 = R1ini;
	u_int R2 = R2ini;
	u_int R3 = R3ini;
	u_int foo = 0;

	u_int base_frame = (first_frame & ~0x0f) + base_shift;
	A5_init(R1, R2, R3, foo, base_frame);

	expand_a5_states(R1, R2, R3, inistate);
	matrix.multiply(inistate, answer);

	for (u_int i = 0; i < A5Matrix::ysize / FRAMESIZE; i++) {
		R1 = R1ini;
		R2 = R2ini;
		R3 = R3ini;
		foo = 0;

		A5_init(R1, R2, R3, foo, first_frame + i);
		u_int shift = first_frame % 16;
		u_int R4 = R4base ^ R4_value(0, base_shift)
			^ R4_value(0, shift + i);
		R4_init(R4);

		for (u_int j = 0; j < FRAMESKIP; j++) {
			A5_gen();
		}
		for (u_int j = 0; (int) j < FRAMESIZE * reverse; j++) {
			A5_gen();
		}
		for (u_int j = 0; j < FRAMESIZE; j++) {
			assert(answer.test(i * FRAMESIZE + j) == A5_gen());
		}
	}
}

static void
fill_matrix_part(A5Matrix &matrix, u_int bitmask, u_int R4, u_int offset, int reverse) {
	reverse = (reverse ? 1 : 0);
	u_int maxstates = FRAMESKIP + FRAMESIZE * (reverse + 1) + R3LEN;
	R1Vec *r1states = new R1Vec[maxstates];
	R2Vec *r2states = new R2Vec[maxstates];
	R3Vec *r3states = new R3Vec[maxstates];

	fill_R1_states(r1states, maxstates, bitmask);
	fill_R2_states(r2states, maxstates, bitmask);
	fill_R3_states(r3states, maxstates, bitmask);

	u_int r1row = 0;
	u_int r2row = 0;
	u_int r3row = 0;

	R4_init(R4);

	for (u_int i = 0; i < FRAMESKIP; i++) {
		clock_rows(r1row, r2row, r3row);
	}

	for (u_int i = 0; (int) i < FRAMESIZE * reverse; i++) {
		clock_rows(r1row, r2row, r3row);
	}

	for (u_int i = 0; i < FRAMESIZE; i++) {
		clock_rows(r1row, r2row, r3row);

		assert(r1row < maxstates);
		assert(r2row < maxstates);
		assert(r3row < maxstates);

		R1Vec r1state(r1states[r1row]);
		r1state ^= r1states[r1row + 3];
		r1state ^= r1states[r1row + 6];

		for (u_int j = 0; j < r1state.size; j++) {
			if (r1state.test(j)) {
				matrix.set(R1_OFFSET + j, i + offset);
			}
		}
		multiply_reg(r1states[r1row + 3], r1states[r1row + 6],
				matrix, i + offset, R1_OFFSET, R1_MULTI_OFFSET);
		multiply_reg(r1states[r1row + 3], r1states[r1row + 4],
				matrix, i + offset, R1_OFFSET, R1_MULTI_OFFSET);
		multiply_reg(r1states[r1row + 4], r1states[r1row + 6],
				matrix, i + offset, R1_OFFSET, R1_MULTI_OFFSET);

		R2Vec r2state(r2states[r2row]);
		r2state ^= r2states[r2row + 8];
		r2state ^= r2states[r2row + 12];

		for (u_int j = 0; j < r2state.size; j++) {
			if (r2state.test(j)) {
				matrix.set(R2_OFFSET + j, i + offset);
			}
		}
		multiply_reg(r2states[r2row + 8], r2states[r2row + 12],
				matrix, i + offset, R2_OFFSET, R2_MULTI_OFFSET);
		multiply_reg(r2states[r2row + 8], r2states[r2row + 5],
				matrix, i + offset, R2_OFFSET, R2_MULTI_OFFSET);
		multiply_reg(r2states[r2row + 5], r2states[r2row + 12],
				matrix, i + offset, R2_OFFSET, R2_MULTI_OFFSET);

		R3Vec r3state(r3states[r3row]);
		r3state ^= r3states[r3row + 4];
		r3state ^= r3states[r3row + 6];

		for (u_int j = 0; j < r3state.size; j++) {
			if (r3state.test(j)) {
				matrix.set(R3_OFFSET + j, i + offset);
			}
		}
		multiply_reg(r3states[r3row + 4], r3states[r3row + 6],
				matrix, i + offset, R3_OFFSET, R3_MULTI_OFFSET);
		multiply_reg(r3states[r3row + 4], r3states[r3row + 9],
				matrix, i + offset, R3_OFFSET, R3_MULTI_OFFSET);
		multiply_reg(r3states[r3row + 9], r3states[r3row + 6],
				matrix, i + offset, R3_OFFSET, R3_MULTI_OFFSET);
	}

	delete [] r1states;
	delete [] r2states;
	delete [] r3states;
}

void
make_a5_matrix(A5Matrix &matrix, u_int R4base, u_int base_offset,
		u_int matrix_offset, int reverse) {
	assert(A5Matrix::ysize % FRAMESIZE == 0);
	assert(base_offset <= 3);
	assert(matrix_offset <= 12);

//	printf("base %d, matrix %d, reverse %d\n", base_offset, matrix_offset,
//			reverse);

	for (u_int i = 0; i < matrix.ysize / FRAMESIZE; i++) {
		u_int frame = matrix_offset + i;
		u_int R4mask = R4_value(0, base_offset)
			^ R4_value(0, matrix_offset + i);
		u_int R4 = R4base ^ R4mask;
		fill_matrix_part(matrix, frame ^ base_offset, R4,
				i * FRAMESIZE, reverse);
	}

//	for (u_int i = 0; i < 100; i++) {
		verify_a5_matrix(matrix, (rand() << 8) + matrix_offset,
				base_offset, R4base, reverse);
//	}
}
