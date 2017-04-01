#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C"
{
#endif

size_t strlen(const char* const str);
bool strcmp(const char* const str1, const char* const str2);
char* tostr(const size_t input, const size_t base);
size_t strparse(const char* const str, const size_t base);
char* strcenter(const char* const str);
char* strjoin(const char* const str1, const char* const str2);

#if defined(__cplusplus)
}
#endif
