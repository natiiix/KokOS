#pragma once

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

int32_t powInt32(const int32_t value, const int32_t power);
double powDouble(const double value, const int32_t power);
int32_t absInt32(const int32_t value);
double absDouble(const double value);

#if defined(__cplusplus)
}
#endif
