#include "cracker.h"
#include "a5.h"

typedef Vec<FRAMELEN> FrameVec;

#define R1TAP 0x72000
#define R2TAP 0x300000
#define R3TAP 0x700080
#define R4TAP 0x10800

#define FULLSTATELEN (R1LEN + R2LEN + R3LEN + R4LEN)

static Matrix<FRAMELEN + FULLSTATELEN, KEYSIZE> solver;
static int solver_initialized = 0;

static void
make_r1_matrix(A5Key *kstates, FrameVec *fstates, u_int count) {
	for (u_int i = 0; i < count; i++) {
		kstates[i].clear();
		fstates[i].clear();

		if (i < KEYSIZE) {
			kstates[i].set(i);
		}
		if (i >= KEYSIZE && (i - KEYSIZE) < FRAMELEN) {
			fstates[i].set(i - KEYSIZE);
		}
		if (i >= 19) {
			kstates[i] ^= kstates[i - 19];
			fstates[i] ^= fstates[i - 19];
		}
		if (i >= 18) {
			kstates[i] ^= kstates[i - 18];
			fstates[i] ^= fstates[i - 18];
		}
		if (i >= 17) {
			kstates[i] ^= kstates[i - 17];
			fstates[i] ^= fstates[i - 17];
		}
		if (i >= 14) {
			kstates[i] ^= kstates[i - 14];
			fstates[i] ^= fstates[i - 14];
		}
	}
}

static void
make_r2_matrix(A5Key *kstates, FrameVec *fstates, u_int count) {
	for (u_int i = 0; i < count; i++) {
		kstates[i].clear();
		fstates[i].clear();

		if (i < KEYSIZE) {
			kstates[i].set(i);
		}
		if (i >= KEYSIZE && (i - KEYSIZE) < FRAMELEN) {
			fstates[i].set(i - KEYSIZE);
		}
		if (i >= 22) {
			kstates[i] ^= kstates[i - 22];
			fstates[i] ^= fstates[i - 22];
		}
		if (i >= 21) {
			kstates[i] ^= kstates[i - 21];
			fstates[i] ^= fstates[i - 21];
		}
	}
}

static void
make_r3_matrix(A5Key *kstates, FrameVec *fstates, u_int count) {
	for (u_int i = 0; i < count; i++) {
		kstates[i].clear();
		fstates[i].clear();

		if (i < KEYSIZE) {
			kstates[i].set(i);
		}
		if (i >= KEYSIZE && (i - KEYSIZE) < FRAMELEN) {
			fstates[i].set(i - KEYSIZE);
		}
		if (i >= 23) {
			kstates[i] ^= kstates[i - 23];
			fstates[i] ^= fstates[i - 23];
		}
		if (i >= 22) {
			kstates[i] ^= kstates[i - 22];
			fstates[i] ^= fstates[i - 22];
		}
		if (i >= 21) {
			kstates[i] ^= kstates[i - 21];
			fstates[i] ^= fstates[i - 21];
		}
		if (i >= 8) {
			kstates[i] ^= kstates[i - 8];
			fstates[i] ^= fstates[i - 8];
		}
	}
}

static void
make_r4_matrix(A5Key *kstates, FrameVec *fstates, u_int count) {
	for (u_int i = 0; i < count; i++) {
		kstates[i].clear();
		fstates[i].clear();

		if (i < KEYSIZE) {
			kstates[i].set(i);
		}
		if (i >= KEYSIZE && (i - KEYSIZE) < FRAMELEN) {
			fstates[i].set(i - KEYSIZE);
		}
		if (i >= 17) {
			kstates[i] ^= kstates[i - 17];
			fstates[i] ^= fstates[i - 17];
		}
		if (i >= 12) {
			kstates[i] ^= kstates[i - 12];
			fstates[i] ^= fstates[i - 12];
		}
	}
}

static void
init_solver_matrix(void) {
	A5Key kstates[KEYSIZE + FRAMELEN];
	FrameVec fstates[KEYSIZE + FRAMELEN];
	Matrix<KEYSIZE + FRAMELEN + FULLSTATELEN, FULLSTATELEN> mat;
	u_int row = 0;

	make_r1_matrix(kstates, fstates, KEYSIZE + FRAMELEN);
	for (u_int i = 0; i < R1LEN; i++, row++) {
		for (u_int j = 0; j < KEYSIZE; j++) {
			if (kstates[KEYSIZE + FRAMELEN - R1LEN + i].test(j)) {
				mat.set(j, row);
			}
		}
		for (u_int j = 0; j < FRAMELEN; j++) {
			if (fstates[KEYSIZE + FRAMELEN - R1LEN + i].test(j)) {
				mat.set(KEYSIZE + j, row);
			}
		}
		mat.set(KEYSIZE + FRAMELEN + row, row);
	}

	make_r2_matrix(kstates, fstates, KEYSIZE + FRAMELEN);
	for (u_int i = 0; i < R2LEN; i++, row++) {
		for (u_int j = 0; j < KEYSIZE; j++) {
			if (kstates[KEYSIZE + FRAMELEN - R2LEN + i].test(j)) {
				mat.set(j, row);
			}
		}
		for (u_int j = 0; j < FRAMELEN; j++) {
			if (fstates[KEYSIZE + FRAMELEN - R2LEN + i].test(j)) {
				mat.set(KEYSIZE + j, row);
			}
		}
		mat.set(KEYSIZE + FRAMELEN + row, row);
	}

	make_r3_matrix(kstates, fstates, KEYSIZE + FRAMELEN);
	for (u_int i = 0; i < R3LEN; i++, row++) {
		for (u_int j = 0; j < KEYSIZE; j++) {
			if (kstates[KEYSIZE + FRAMELEN - R3LEN + i].test(j)) {
				mat.set(j, row);
			}
		}
		for (u_int j = 0; j < FRAMELEN; j++) {
			if (fstates[KEYSIZE + FRAMELEN - R3LEN + i].test(j)) {
				mat.set(KEYSIZE + j, row);
			}
		}
		mat.set(KEYSIZE + FRAMELEN + row, row);
	}

	make_r4_matrix(kstates, fstates, KEYSIZE + FRAMELEN);
	for (u_int i = 0; i < R4LEN; i++, row++) {
		for (u_int j = 0; j < KEYSIZE; j++) {
			if (kstates[KEYSIZE + FRAMELEN - R4LEN + i].test(j)) {
				mat.set(j, row);
			}
		}
		for (u_int j = 0; j < FRAMELEN; j++) {
			if (fstates[KEYSIZE + FRAMELEN - R4LEN + i].test(j)) {
				mat.set(KEYSIZE + j, row);
			}
		}
		mat.set(KEYSIZE + FRAMELEN + row, row);
	}
	for (u_int i = 0; i < mat.xsize; i++) {
		mat.reset(i, R1LEN - 1 - 15);
		mat.reset(i, R1LEN + R2LEN - 1 - 16);
		mat.reset(i, R1LEN + R2LEN + R3LEN - 1 - 18);
		mat.reset(i, R1LEN + R2LEN + R3LEN + R4LEN - 1 - 10);
	}

	mat.solve(KEYSIZE);

	for (u_int i = 0; i < KEYSIZE; i++) {
		for (u_int j = 0; j < FRAMELEN + FULLSTATELEN; j++) {
			if (mat.test(KEYSIZE + j, i)) {
				solver.set(j, i);
			}
		}
	}
	solver_initialized = 1;
}

void
recover_key(u_int R1, u_int R2, u_int R3, u_int R4, u_int frame, A5Key &key) {
	Vec<FRAMELEN + FULLSTATELEN> state;
	u_int bit = 0;

	if (! solver_initialized) {
		init_solver_matrix();
	}

	for (u_int i = 0; i < FRAMELEN; i++, bit++) {
		if ((frame >> i) & 1) {
			state.flip(bit);
		}
	}
	for (u_int i = 0; i < R1LEN; i++, bit++) {
		if ((R1 >> (R1LEN - 1 - i)) & 1) {
			state.flip(bit);
		}
	}
	for (u_int i = 0; i < R2LEN; i++, bit++) {
		if ((R2 >> (R2LEN - 1 - i)) & 1) {
			state.flip(bit);
		}
	}
	for (u_int i = 0; i < R3LEN; i++, bit++) {
		if ((R3 >> (R3LEN - 1 - i)) & 1) {
			state.flip(bit);
		}
	}
	for (u_int i = 0; i < R4LEN; i++, bit++) {
		if ((R4 >> (R4LEN - 1 - i)) & 1) {
			state.flip(bit);
		}
	}

	solver.multiply(state, key);
}
