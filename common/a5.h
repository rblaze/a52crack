#pragma once

#include "matrix.h"

#define FRAMELEN 22
#define KEYSIZE 64

typedef Vec<KEYSIZE> A5Key;

/** Initialize A5/2 states. */
void A5_init(u_int &R1, u_int &R2, u_int &R3, u_int &R4, u_int frame);

/** Initialize A5/2 states from key */
void A5_init(const A5Key &key, u_int frame);

/** Generate A5/2 bit. */
int A5_gen(void);

/** Initialize R4 state. */
void R4_init(u_int R4);

/** Calculate R4 value. */
u_int R4_value(u_int R4, u_int frame);

/** Clock A5/2 registers */
void R4clock(int &clock_R1, int &clock_R2, int &clock_R3);

/** Generate A5/2 stream. */
template <u_int size>
void A5_stream(u_int R1, u_int R2, u_int R3, u_int R4, u_int frame,
		u_int skip, Vec<size> &stream) {
	stream.clear();
	A5_init(R1, R2, R3, R4, frame);
	for (u_int i = 0; i < skip; i++) {
		A5_gen();
	}
	for (u_int i = 0; i < size; i++) {
		if (A5_gen()) {
			stream.set(i);
		}
	}
}

/** Generate A5/2 stream from key */
template <u_int size>
void A5_stream(const A5Key &key, u_int frame, u_int skip, Vec<size> &stream) {
	stream.clear();
	A5_init(key, frame);
	for (u_int i = 0; i < skip; i++) {
		A5_gen();
	}
	for (u_int i = 0; i < size; i++) {
		if (A5_gen()) {
			stream.set(i);
		}
	}
}
