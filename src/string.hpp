#pragma once

#include "stdtypes.hpp"
#include "memory.hpp"

namespace str
{
    size_t len(const char* str);
    bool cmp(const char* str1, const char* str2);
    char* convert(const size_t input, const size_t base = 10);
}
