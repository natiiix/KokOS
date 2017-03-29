#pragma once

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

void mem_init(void);
size_t mem_used(void);
size_t mem_empty(void);
void* alloc(const size_t length);
void free(const void* const ptr);
void* mem_copy(const void* const ptrsrc, const size_t length);
void* dynalloc(const size_t initsize);
void* dynresize(void* const ptr, const size_t newsize);

void* memset(void* ptr, int value, size_t num);

void* phystovirt(const size_t physAddr);
void* phystovirtptr(const void* const physAddr);
void* virttophys(const size_t virtAddr);
void* virttophysptr(const void* const virtAddr);

uint32_t low32(const uint64_t value);
uint32_t high32(const uint64_t value);

#if defined(__cplusplus)
}

inline void* operator new(size_t count)
{ return alloc(count); }

inline void operator delete(void* ptr)
{ free(ptr); }
#endif
