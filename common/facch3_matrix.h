#pragma once

#include "matrix.h"
#include "facch3_encoder.h"

/* FACCH3 message is 76 bits long, but to implement scrambler
 * we need additional bit always set to 1
 */

typedef Matrix<FACCH3_MSG_SIZE + 1, FACCH3_OUTPUT_SIZE> Facch3Matrix;

void make_facch3_matrix(Facch3Matrix &matrix);
