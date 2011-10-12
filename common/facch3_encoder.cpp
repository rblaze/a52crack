#include "facch3_encoder.h"
#include "crc16.h"

void
rate14encoder(const CrcMessage &in, Rate14Message &out) {
	const u_int MASK = 0x1f;	/* 11111 */
	const u_int P1 = 0x19;		/* 11001 */
	const u_int P2 = 0x17;		/* 10111 */
	const u_int P3 = 0x15;		/* 10101 */
	const u_int P4 = 0x1f;		/* 11111 */

	u_int reg = 0;
	out.clear();

	for (u_int i = 0; i < in.size + 4; i++) {
		reg <<= 1;
		if (i < in.size) {
			reg |= in.test(i);
		}
		reg &= MASK;
		if (wordparity(reg & P1)) {
			out.set(i * 4);
		}
		if (wordparity(reg & P2)) {
			out.set(i * 4 + 1);
		}
		if (wordparity(reg & P3)) {
			out.set(i * 4 + 2);
		}
		if (wordparity(reg & P4)) {
			out.set(i * 4 + 3);
		}
	}
}

void
splitter(const Rate14Message &in, Facch3OutputBlock &out) {
	assert(in.size == out.size);
	out.clear();
	for (u_int n = 0; n < FACCH3_BLOCK_SIZE; n++) {
		u_int i = n % 12;
		u_int j = n / 12;
		u_int k = i * 8 + (j * 5) % 8;

		for (u_int x = 0; x < 4; x++) {
			if (in.test(k * 4 + x)) {
				out.set(x * FACCH3_BLOCK_SIZE + n);
			}
		}
	}
}

void
scrambler(Facch3OutputBlock &data) {
	u_int mask = 0x7fff;	/* 15-bit LFSR */
	u_int tap = 0x4001;		/* with 1+D+D15 polynomial */
	u_int reg = 0x4D4B;		/* and 1+D+D3+D6+D8+D10+D11+D14 initial value */

	for (u_int i = 0; i < FACCH3_BLOCK_SIZE; i++) {
		u_int t = wordparity(reg & tap);

		if (t) {
			data.flip(i);
			data.flip(FACCH3_BLOCK_SIZE + i);
			data.flip(FACCH3_BLOCK_SIZE * 2 + i);
			data.flip(FACCH3_BLOCK_SIZE * 3 + i);
		}
		reg = (reg << 1) & mask;
		reg |= t;
	}
}

void
facch3_encoder(const Facch3Msg &in, Facch3OutputBlock &out) {
	u_short crc = calc_crc(in);
	CrcMessage crc_message(in);

	for (u_int i = 0; i < CRCBITS; i++) {
		if (crc & (1 << (CRCBITS - 1 - i))) {
			crc_message.set(in.size + i);
		}
	}

	Rate14Message rate14_message;
	rate14encoder(crc_message, rate14_message);

	splitter(rate14_message, out);
	scrambler(out);
}
