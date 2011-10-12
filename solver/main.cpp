#include "common.h"
#include "a5.h"
#include "cracker.h"
#include <ctime>

static void
test_data(void) {
	A5Key key;

	key.fill_random();

	FrameInfo data[SOLUTION_FRAMES];
	data[0].frame_number = random() & 0x3FFFF3;
	data[0].reverse = 0;
	data[1].frame_number = data[0].frame_number + 4;
	data[1].reverse = 0;
	data[2].frame_number = data[0].frame_number + 8;
	data[2].reverse = 0;

	for (u_int n = 0; n < SOLUTION_FRAMES; n++) {
		Facch3Msg msg;

		msg.fill_random();
		facch3_encoder(msg, data[n].data);

		for (u_int j = 0; j < 4; j++) {
			A5_init(key, data[0].frame_number + 4 * n + j);

			for (u_int k = 0; k < 99; k++) {
				A5_gen();
			}
			for (u_int k = 0; k < FACCH3_BLOCK_SIZE; k++) {
				if (A5_gen()) {
					data[n].data.flip(j * FACCH3_BLOCK_SIZE + k);
				}
			}
		}
	}

	Facch3OutputBlock testdata[3];
	testdata[0] = data[0].data;
	testdata[1] = data[1].data;
	testdata[2] = data[2].data;

	printf("frame %08x\n", data[0].frame_number);
	u_int R4v = lookup_R4_value(data[0].frame_number, testdata);
	u_int R1v, R2v, R3v;
	get_ini_values(data, R4v, R1v, R2v, R3v);

	A5Key recovered_key;
	recover_key(R1v, R2v, R3v, R4v, data[0].frame_number, recovered_key);

	if (! (key == recovered_key)) {
		printf("TEST FAILED\n");
		exit(1);
	}
}

int
main(void) {
	srandom(time(NULL));
	crc_selftest();
	a5_selftest();

	for (u_int i = 0; i < 50; i++) {
		test_data();
	}
	printf("all tests passed\n");
	return 0;
}
