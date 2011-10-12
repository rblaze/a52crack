#include "a5.h"
#include "common.h"

#define R1TAP 0x72000
#define R2TAP 0x300000
#define R3TAP 0x700080

#define R4TAP 0x10800

#define R4CLOCK1 0x000400
#define R4CLOCK2 0x000008
#define R4CLOCK3 0x000080

static u_int R1state, R2state, R3state, R4state;
static int R1bit, R2bit, R3bit;

/* Return 1 if at least two of the parameter words are non-zero. */
static u_int
majority(u_int w1, u_int w2, u_int w3) {
	int sum = (w1 != 0) + (w2 != 0) + (w3 != 0);
	if (sum >= 2)
		return 1;
	else
		return 0;
}

static u_int
R1gen(void) {
	u_int t = R1state & R1TAP;		// wanted bits

	u_int rv = (R1state >> 18) ? 1 : 0;
	rv ^= majority(R1state & 0x8000, R1state & 0x1000, (~R1state) & 0x4000);

	R1state = (R1state << 1) & R1MASK;
	R1state |= wordparity(t);

	return rv;
}

static u_int
R2gen(void) {
	u_int t = R2state & R2TAP;		// wanted bits

	u_int rv = (R2state >> 21) ? 1 : 0;
	rv ^= majority(R2state & 0x2000, R2state & 0x200, (~R2state) & 0x10000);

	R2state = (R2state << 1) & R2MASK;
	R2state |= wordparity(t);

	return rv;
}

static u_int
R3gen(void) {
	u_int t = R3state & R3TAP;		// wanted bits

	u_int rv = (R3state >> 22) ? 1 : 0;
	rv ^= majority(R3state & 0x40000, R3state & 0x10000, (~R3state) & 0x2000);

	R3state = (R3state << 1) & R3MASK;
	R3state |= wordparity(t);

	return rv;
}

/** R4 clocking generator */
void
R4clock(int &clock_R1, int &clock_R2, int &clock_R3) {
	u_int t = R4state & R4TAP;		// wanted bits
	u_int maj = majority(R4state & R4CLOCK1, R4state & R4CLOCK2,
			R4state & R4CLOCK3);

	clock_R1 = ((R4state & R4CLOCK1) != 0) == maj;
	clock_R2 = ((R4state & R4CLOCK2) != 0) == maj;
	clock_R3 = ((R4state & R4CLOCK3) != 0) == maj;

	R4state = (R4state << 1) & R4MASK;
	R4state |= wordparity(t);
}

static void
clock_regs(void) {
	int clock_R1, clock_R2, clock_R3;

	R4clock(clock_R1, clock_R2, clock_R3);

	if (clock_R1) {
		R1bit = R1gen();
	}
	if (clock_R2) {
		R2bit = R2gen();
	}
	if (clock_R3) {
		R3bit = R3gen();
	}
}

u_int
R4_value(u_int R4, u_int frame) {
	u_int state = R4 & R4MASK;

	for (u_int i = 0; i < FRAMELEN; i++) {
		u_int t = state & R4TAP;
		state = (state << 1) & R4MASK;
		state |= wordparity(t);
		if ((frame >> i) & 1) {
			state ^= 1;
		}
	}
	state |= 1 << 10;

	return state;
}

void
R4_init(u_int R4) {
	assert(R4 & (1 << 10));
	R4state = R4;
}

void
A5_init(const A5Key &key, u_int frame) {
	R1state = 0;
	R2state = 0;
	R3state = 0;
	R4state = 0;

	for (u_int i = 0; i < KEYSIZE; i++) {
		R1gen();
		R2gen();
		R3gen();
		u_int t = R4state & R4TAP;
		R4state = (R4state << 1) & R4MASK;
		R4state |= wordparity(t);
		if (key.test(i)) {
			R1state ^= 1;
			R2state ^= 1;
			R3state ^= 1;
			R4state ^= 1;
		}
	}

	for (u_int i = 0; i < FRAMELEN; i++) {
		R1gen();
		R2gen();
		R3gen();
		u_int t = R4state & R4TAP;
		R4state = (R4state << 1) & R4MASK;
		R4state |= wordparity(t);
		if ((frame >> i) & 1) {
			R1state ^= 1;
			R2state ^= 1;
			R3state ^= 1;
			R4state ^= 1;
		}
	}
	R1state |= 1 << 15;
	R2state |= 1 << 16;
	R3state |= 1 << 18;
	R4state |= 1 << 10;

	R1bit = R1gen();
	R2bit = R2gen();
	R3bit = R3gen();
}

void
A5_init(u_int &R1, u_int &R2, u_int &R3, u_int &R4, u_int frame) {
	R1state = R1 & R1MASK;
	R2state = R2 & R2MASK;
	R3state = R3 & R3MASK;
	R4state = R4 & R4MASK;

	for (u_int i = 0; i < FRAMELEN; i++) {
		R1gen();
		R2gen();
		R3gen();
		u_int t = R4state & R4TAP;
		R4state = (R4state << 1) & R4MASK;
		R4state |= wordparity(t);
		if ((frame >> i) & 1) {
			R1state ^= 1;
			R2state ^= 1;
			R3state ^= 1;
			R4state ^= 1;
		}
	}
	R1state |= 1 << 15;
	R2state |= 1 << 16;
	R3state |= 1 << 18;
	R4state |= 1 << 10;

	R1 = R1state;
	R2 = R2state;
	R3 = R3state;
	R4 = R4state;

	R1bit = R1gen();
	R2bit = R2gen();
	R3bit = R3gen();
}

int
A5_gen(void) {
	clock_regs();
	return (R1bit ^ R2bit ^ R3bit);
}
