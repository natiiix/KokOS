#pragma once

#include "stdtypes.hpp"

namespace mem
{
    void init(void);
    void* alloc(const size_t bytes);
    void free(void* const ptr);
    void free(void* const ptr, const size_t bytes);
}
