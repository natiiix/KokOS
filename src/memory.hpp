#pragma once

#include "stdtypes.hpp"
#include "string.hpp"

namespace mem
{
    void init(void);
    void* alloc(const size_t bytes);
    void free(const void* const ptr, const size_t bytes);
    void free(const void* const ptr);
    void* copy(const void* const ptrsrc, const size_t bytes);
    void* copy(const void* const ptrsrc);
    size_t used(void);
    size_t empty(void);
}
