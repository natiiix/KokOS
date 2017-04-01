#pragma once

#include <stddef.h>
#include <stdint.h>

size_t mem_used(void);
size_t mem_empty(void);
void* mem_alloc(const size_t length);
void mem_free(const void* const ptr);
void* mem_copy(const void* const ptrsrc, const size_t length);
void* mem_dynalloc(const size_t initsize);
void* mem_dynresize(void* const ptr, const size_t newsize);
void* mem_set(void* ptr, int value, size_t num);
