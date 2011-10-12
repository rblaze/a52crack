#include "crc16.h"
#include "facch3_encoder.h"
#include "facch3_matrix.h"

/* CRC and 1/4 encoder matrices take one extra bit, last one */

typedef Matrix<FACCH3_MSG_SIZE + 1, CRCMSG_SIZE + 1> CrcMatrix;
typedef Matrix<CRCMSG_SIZE + 1, RATE14MSG_SIZE + 1> EncoderMatrix;
typedef Matrix<RATE14MSG_SIZE + 1, FACCH3_OUTPUT_SIZE> SplitScramblerMatrix;

static void
make_crc_matrix(CrcMatrix &matrix) {
	for (u_int i = 0; i < FACCH3_MSG_SIZE; i++) {
		/* Output message bit in start of message */
		matrix.set(i, i);

		/* Mark dependent CRC bits */
		Facch3Msg stream;
		stream.set(i);
		
		u_short poly = calc_crc(stream);
		for (u_int j = 0; j < CRCBITS; j++) {
			if (poly & (1 << (CRCBITS - 1 - j))) {
				matrix.set(i, FACCH3_MSG_SIZE + j);
			}
		}
	}
	matrix.set(matrix.xsize - 1, matrix.ysize - 1);
}

static void
verify_crc_matrix(const CrcMatrix &matrix) {
	Facch3Msg testvec;
	Vec<CRCMSG_SIZE + 1> answer;

	for (u_int i = 0; i < 1000; i++) {
		testvec.fill_random();
		u_short crc = calc_crc(testvec);

		Vec<FACCH3_MSG_SIZE + 1> data(testvec);
		matrix.multiply(data, answer);
		u_short ans = 0;
		for (u_int j = 0; j < CRCBITS; j++) {
			ans |= answer.test(j + FACCH3_MSG_SIZE) << (CRCBITS - 1 - j);
		}
		assert(crc == ans);
		crc = 0;
	}
}

static void
make_14encoder_matrix(EncoderMatrix &matrix) {
	for (u_int i = 0; i < CRCMSG_SIZE + 4; i++) {
		/* k */
		if (i < CRCMSG_SIZE) {
			matrix.set(i, 4 * i);
			matrix.set(i, 4 * i + 1);
			matrix.set(i, 4 * i + 2);
			matrix.set(i, 4 * i + 3);
		}
		/* k - 1 */
		if (i > 0 && i < CRCMSG_SIZE + 1) {
			matrix.set(i - 1, 4 * i + 1);
			matrix.set(i - 1, 4 * i + 3);
		}
		/* k - 2 */
		if (i > 1 && i < CRCMSG_SIZE + 2) {
			matrix.set(i - 2, 4 * i + 1);
			matrix.set(i - 2, 4 * i + 2);
			matrix.set(i - 2, 4 * i + 3);
		}
		/* k - 3 */
		if (i > 2 && i < CRCMSG_SIZE + 3) {
			matrix.set(i - 3, 4 * i);
			matrix.set(i - 3, 4 * i + 3);
		}
		/* k - 4 */
		if (i > 3) {
			matrix.set(i - 4, 4 * i);
			matrix.set(i - 4, 4 * i + 1);
			matrix.set(i - 4, 4 * i + 2);
			matrix.set(i - 4, 4 * i + 3);
		}
	}
	matrix.set(matrix.xsize - 1, matrix.ysize - 1);
}

static void
verify_14encoder_matrix(const EncoderMatrix &matrix) {
	CrcMessage testvec;
	Rate14Message answer;

	for (u_int i = 0; i < 1000; i++) {
		testvec.fill_random();
		rate14encoder(testvec, answer);

		Vec<CRCMSG_SIZE + 1> data(testvec);
		Vec<RATE14MSG_SIZE + 1> enc_answer(answer), matrix_answer;
		matrix.multiply(data, matrix_answer);
		assert(enc_answer == matrix_answer);
	}
}

static void
make_split_scrambler_matrix(SplitScramblerMatrix &matrix) {
	Facch3OutputBlock stream;
	scrambler(stream);

	for (u_int n = 0; n < FACCH3_BLOCK_SIZE; n++) {
		u_int i = n % 12;
		u_int j = n / 12;
		u_int k = i * 8 + (j * 5) % 8;

		for (u_int x = 0; x < 4; x++) {
			/* split message */
			matrix.set(k * 4 + x, x * FACCH3_BLOCK_SIZE + n);
			/* flip some bits for scrambling */
			if (stream.test(n)) {
				matrix.set(matrix.xsize - 1, x * FACCH3_BLOCK_SIZE + n);
			}
		}
	}
}

static void
verify_split_scrambler_matrix(const SplitScramblerMatrix &matrix) {
	Rate14Message testvec;
	Facch3OutputBlock answer;
	Facch3OutputBlock matrix_answer;

	for (u_int i = 0; i < 1000; i++) {
		testvec.fill_random();
		splitter(testvec, answer);
		scrambler(answer);

		Vec<RATE14MSG_SIZE + 1> data(testvec);
		data.set(data.size - 1);
		matrix.multiply(data, matrix_answer);
		assert(answer == matrix_answer);
	}
}

static void
verify_final_matrix(const Facch3Matrix &matrix) {
	Facch3Msg testvec;
	Facch3OutputBlock answer;
	Facch3OutputBlock matrix_answer;

	for (u_int i = 0; i < 1000; i++) {
		testvec.fill_random();
		facch3_encoder(testvec, answer);

		Vec<FACCH3_MSG_SIZE + 1> data(testvec);
		data.set(data.size - 1);
		matrix.multiply(data, matrix_answer);
		assert(answer == matrix_answer);
	}
}

void
make_facch3_matrix(Facch3Matrix &matrix) {
	CrcMatrix crc_matrix;
	EncoderMatrix enc_matrix;
	SplitScramblerMatrix ss_matrix;

	make_crc_matrix(crc_matrix);
	verify_crc_matrix(crc_matrix);

	make_14encoder_matrix(enc_matrix);
	verify_14encoder_matrix(enc_matrix);

	make_split_scrambler_matrix(ss_matrix);
	verify_split_scrambler_matrix(ss_matrix);

	Matrix<FACCH3_MSG_SIZE + 1, RATE14MSG_SIZE + 1> crc_enc;
	crc_enc.make_product(crc_matrix, enc_matrix);
	matrix.make_product(crc_enc, ss_matrix);

	verify_final_matrix(matrix);
}
