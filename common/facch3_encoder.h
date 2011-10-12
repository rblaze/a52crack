#pragma once

#include "matrix.h"

#define FACCH3_MSG_SIZE 76
#define FACCH3_BLOCK_SIZE 96
#define FACCH3_OUTPUT_SIZE (FACCH3_BLOCK_SIZE * 4)
#define CRCBITS 16
#define CRCMSG_SIZE (FACCH3_MSG_SIZE + CRCBITS)
#define RATE14MSG_SIZE (CRCMSG_SIZE * 4 + 16)

typedef Vec<FACCH3_MSG_SIZE> Facch3Msg;
typedef Vec<FACCH3_OUTPUT_SIZE> Facch3OutputBlock;
typedef Vec<CRCMSG_SIZE> CrcMessage;
typedef Vec<RATE14MSG_SIZE> Rate14Message;

void facch3_encoder(const Facch3Msg &in, Facch3OutputBlock &out);
void rate14encoder(const CrcMessage &in, Rate14Message &out);
void splitter(const Rate14Message &in, Facch3OutputBlock &out);
void scrambler(Facch3OutputBlock &data);
