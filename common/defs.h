#pragma once

#include <sys/types.h>
#include <sys/param.h>
#include <arpa/inet.h>

#define WORDBITS (sizeof(u_int32_t) * 8)
#define wordparity(x) __builtin_parity(x)
