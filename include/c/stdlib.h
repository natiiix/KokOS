#pragma once

#include <stddef.h>

#if defined(__cplusplus)
extern "C"
{
#endif

void free(const void* const ptr);
void* malloc(const size_t size);
void* calloc(const size_t size);
void* realloc(void* const ptr, size_t size);

#if defined(__cplusplus)
}

inline void operator delete(void* ptr)
{
    free(ptr);
}
#endif
