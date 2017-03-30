#pragma once

#include <stddef.h>
#include <stdint.h>

void mem_init(void);
size_t mem_used(void);
size_t mem_empty(void);
void* mem_alloc(const size_t length);
void mem_free(const void* const ptr);
void* mem_copy(const void* const ptrsrc, const size_t length);
void* mem_dynalloc(const size_t initsize);
void* mem_dynresize(void* const ptr, const size_t newsize);

void* mem_set(void* ptr, int value, size_t num);
void* mem_phystovirt(const size_t physAddr);
void* mem_phystovirtptr(const void* const physAddr);
void* mem_virttophys(const size_t virtAddr);
void* mem_virttophysptr(const void* const virtAddr);

uint32_t low32(const uint64_t value);
uint32_t high32(const uint64_t value);
