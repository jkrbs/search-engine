#pragma once
#include <stdbool.h>
typedef void* math_posting_t;
#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "Not little-endian machine."
#endif
