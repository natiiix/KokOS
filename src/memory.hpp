#pragma once

#include "stdtypes.hpp"
#include "string.hpp"

namespace mem
{
    void init(void);
    size_t used(void);
    size_t empty(void);
    void* alloc(const size_t length);
    void free(const void* const ptr);
    void* copy(const void* const ptrsrc, const size_t length);
    void* copy(const char* const str);
}
