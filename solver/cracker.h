#pragma once

#include "common.h"
#include "a5.h"

/* Data requirements:
 * 1) 3 FACCH3 messages in row, with sequential frame numbers (12 frames)
 * 2) First frame number & 0x0f <= 3
 * 3) Frames must be in "forward" direction
 */
u_int lookup_R4_value(u_int frame_number, const Facch3OutputBlock data[3]);

struct FrameInfo {
	u_int frame_number;
	int reverse;
	Facch3OutputBlock data;
};

#define SOLUTION_FRAMES 3

/* Data requirements:
 * 1) FACCH3 frames, each in 4 sequential frames.
 * 2) First frame number must be same as used in lookup_R4_value()
 * 3) Frame numbers xor differences must be less than 0x200
 */

void get_ini_values(const FrameInfo data[SOLUTION_FRAMES], u_int R4,
		u_int &R1, u_int &R2, u_int &R3);


void recover_key(u_int R1, u_int R2, u_int R3, u_int R4, u_int frame,
		A5Key &key);
