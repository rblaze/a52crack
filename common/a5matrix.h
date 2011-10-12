#pragma once

#include "facch3_encoder.h"

#define R1LEN 19
#define R2LEN 22
#define R3LEN 23
#define R4LEN 17
#define MULTIBITS(x) (x * (x -1) / 2)
#define A5SRC_LEN (R1LEN + R2LEN + R3LEN + \
		MULTIBITS(R1LEN) + MULTIBITS(R2LEN) + MULTIBITS(R3LEN))

#define R1_MULTI_OFFSET 0
#define R2_MULTI_OFFSET MULTIBITS(R1LEN)
#define R3_MULTI_OFFSET (R2_MULTI_OFFSET + MULTIBITS(R2LEN))
#define R1_OFFSET (R3_MULTI_OFFSET + MULTIBITS(R3LEN))
#define R2_OFFSET (R1_OFFSET + R1LEN)
#define R3_OFFSET (R2_OFFSET + R2LEN)

typedef Vec<A5SRC_LEN> A5State;
typedef RotatedMatrix<A5SRC_LEN, FACCH3_OUTPUT_SIZE> A5Matrix;

/** Calculate A5 matrix for given R4 initial value.
 * \param R4 register value, assuming frame number already mixed in.
 * \param base_offset first frame offset in 16-frame block
 * \param matrix_offset current matrix offset in 16-frame block
 * \param reverse generate matrix in forward or reverse direction
 */
void make_a5_matrix(A5Matrix &matrix, u_int R4, u_int base_offset,
		u_int matrix_offset, int reverse);
