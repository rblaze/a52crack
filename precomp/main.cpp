#include "common.h"
#include <cstdio>
#include <ctime>

KgMatrix Kg;

static void
calculate_matrix(u_int R4, u_int offset, FILE *file) {
	assert(offset <= 3);

	auto GS = new Matrix<A5SRC_LEN + 3 * KGY, 3 * KGY>();

	for (u_int n = 0; n < 3; n++) {
		A5Matrix S;
		make_a5_matrix(S, R4, offset, n * 4 + offset, 0);

		Matrix<A5SRC_LEN, KGY> mat2;
		mat2.make_product(S, Kg);
		for (u_int j = 0; j < KGY; j++) {
			for (u_int i = 0; i < A5SRC_LEN; i++) {
				if (mat2.test(i, j)) {
					GS->set(i, n * KGY + j);
				}
			}
		}
	}
	for (u_int i = 0; i < 3 * KGY; i++) {
		GS->set(A5SRC_LEN + i, i);
	}
	GS->gauss(A5SRC_LEN);

	CheckMatrix KSD;
	for (u_int j = 0; j < CHECK_EQS; j++) {
		for (u_int i = 0; i < 3 * KGY; i++) {
			if (GS->test(A5SRC_LEN + i, A5SRC_LEN + j)) {
				KSD.set(i, j);
			}
		}
	}
	delete GS;

	fwrite(&R4, sizeof(R4), 1, file);
	fwrite(&KSD, sizeof(KSD), 1, file);
}

int
main(int argc, const char *argv[]) {
	srandom(time(NULL));
	crc_selftest();
	a5_selftest();

	if (argc < 2 || *(argv[1]) < '0' || *(argv[1]) > '3') {
		printf("Usage: precomp {0-3}\n");
		exit(1);
	}
	u_int offset = *(argv[1]) - '0';
	printf("calculating data for offset %d\n", offset);

	init_Kg(Kg);

	FILE *file = fopen(matrixnames[offset], "w");
	if (! file) {
		perror("fopen");
		exit(1);
	}
	for (u_int R4 = (1 << 10); R4 < 0x20000; R4++) {
//	for (u_int R4 = (1 << 10); R4 < 0x1000; R4++) {
		if ((R4 & (1 << 10)) == 0) {
			continue;
		}
		if (R4 % 1000 == 0) {
			printf("%08x\n", R4);
		}
		calculate_matrix(R4, offset, file);
	}
	fclose(file);

	return 0;
}
