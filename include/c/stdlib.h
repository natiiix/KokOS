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

// Stores an object in memory and returns a pointer to it
template<class T>
inline void* memstore(const T value)
{
    // Make space in memory for the object
    T* ptr = (T*)malloc(sizeof(T));
    // Copy the object to the allocated memory space
    (*ptr) = value;
    // Return pointer to the value stored in memory
    return ptr;
}
#endif
