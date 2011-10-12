#include "cracker.h"
#include "cracker_var.h"

KgMatrix Kg;
int Kg_initialized = 0;

u_int
lookup_R4_value(u_int frame_number, const Facch3OutputBlock data[3]) {
	Vec<3 * KGY> outdata;
	u_int offset = frame_number % 4;

	if (! Kg_initialized) {
		init_Kg(Kg);
		Kg_initialized = 1;
	}

	for (u_int i = 0; i < 3; i++) {
		Vec<KGY> tmp;

		Kg.multiply(data[i], tmp);
		for (u_int j = 0; j < KGY; j++) {
			if (tmp.test(j)) {
				outdata.flip(i * KGY + j);
			}
		}
	}

	FILE *mt = fopen(matrixnames[offset], "r");
	if (! mt) {
		perror("fopen");
		exit(1);
	}

	CheckMatrix KSD;
	u_int R4;
	while (! feof(mt)) {
		if (fread(&R4, sizeof(R4), 1, mt) != 1) {
			perror("fread(R4)");
			exit(1);
		}
		if (fread(&KSD, sizeof(KSD), 1, mt) != 1) {
			perror("fread(KSD)");
			exit(1);
		}
		if (R4 % 1000 == 0) {
			printf("%08x\n", R4);
		}

		if (KSD.zero_product(outdata)) {
			fclose(mt);
			return R4;
		}
	}

	printf("R4 not found\n");
	exit(2);
}
