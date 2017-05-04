#pragma once

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

int32_t powInt32(const int32_t value, const size_t power);
double powDouble(const double value, const size_t power);

#if defined(__cplusplus)
}
#endif
