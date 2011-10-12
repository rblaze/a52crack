#include "matrix.h"
#include "facch3_matrix.h"
#include "a5matrix.h"

/* (Kg*S) size */
#define KGY (FACCH3_OUTPUT_SIZE - FACCH3_MSG_SIZE - 1)

#define R1MASK 0x7FFFF
#define R2MASK 0x3FFFFF
#define R3MASK 0x7FFFFF
#define R4MASK 0x1FFFF

#define CHECK_EQS 30

typedef Matrix<FACCH3_OUTPUT_SIZE, KGY> KgMatrix;
typedef Matrix<3 * KGY, CHECK_EQS> CheckMatrix;

void init_Kg(KgMatrix &matrix);

void crc_selftest(void);
void a5_selftest(void);

extern const char *matrixnames[4];
