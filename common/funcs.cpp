#include "common.h"
#include "a5.h"
#include "crc16.h"

const char *matrixnames[4] = {
	"/tmp/s0matrices",
	"/tmp/s1matrices",
	"/tmp/s2matrices",
	"/tmp/s3matrices"
};

void
init_Kg(KgMatrix &matrix) {
	Facch3Matrix enc_matrix;
	make_facch3_matrix(enc_matrix);

	Matrix<FACCH3_MSG_SIZE + 1 + FACCH3_OUTPUT_SIZE, FACCH3_OUTPUT_SIZE> m;
	for (u_int j = 0; j < enc_matrix.ysize; j++) {
		for (u_int i = 0; i < enc_matrix.xsize; i++) {
			if (enc_matrix.test(i, j)) {
				m.set(i, j);
			}
		}
		m.set(FACCH3_MSG_SIZE + 1 + j, j);
	}

	/* empirical constant */
	m.gauss(FACCH3_OUTPUT_SIZE - 2);

	for (u_int j = 0; j < KGY; j++) {
		for (u_int i = 0; i < FACCH3_OUTPUT_SIZE; i++) {
			if (m.test(FACCH3_MSG_SIZE + 1 + i,
						FACCH3_MSG_SIZE + 1 + j)) {
				matrix.set(i, j);
			}
		}
	}
}

void
crc_selftest(void) {
	Vec<72> test("123456789");
	u_short crc = calc_crc(test);
	assert(crc == 0x31C3);
	crc = 0;
	printf("CRC selftest ok\n");
}

void
a5_selftest(void) {
	const unsigned char goodAtoB[15] = {
		0xf4, 0x51, 0x2c, 0xac, 0x13, 0x59, 0x37, 0x64,
		0x46, 0x0b, 0x72, 0x2d, 0xad, 0xd5, 0x00
	};
	const unsigned char goodBtoA[15] = {
		0x48, 0x00, 0xd4, 0x32, 0x8e, 0x16, 0xa1, 0x4d,
		0xcd, 0x7b, 0x97, 0x22, 0x26, 0x51, 0x00
	};
//	const u_char key[8] = {0x00, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	const u_char key[8] = {0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	const u_int testframe = 0x21;

	Vec<114> sampleAB(goodAtoB), sampleBA(goodBtoA), keystream;
	A5Key testkey(key);

	A5_stream(testkey, testframe, 99, keystream);
	assert(sampleAB == keystream);

	A5_stream(testkey, testframe, 99 + 114, keystream);
	assert(sampleBA == keystream);

	printf("A5 selftest ok\n");
}

