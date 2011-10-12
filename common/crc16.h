#pragma once

#include "matrix.h"

template <u_int size>
u_short
calc_crc(const Vec<size> &data) {
	u_short reg = 0;

	for (u_int i = 0; i < size; i++) {
		int flop = reg & 0x8000;
		reg <<= 1;
		reg |= data.test(i) ? 1 : 0;
		if (flop) {
			reg ^= 0x1021;
		}
	}

	for (u_int i = 0; i < 16; i++) {
		int flop = reg & 0x8000;
		reg <<= 1;
		if (flop) {
			reg ^= 0x1021;
		}
	}
	return reg;
}
