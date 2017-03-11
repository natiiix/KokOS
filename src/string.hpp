#pragma once

#include "stdtypes.hpp"
#include "memory.hpp"

namespace str
{
    size_t len(const char* str);
    char* convert(const size_t input);
    void convertat(const size_t input, char* output);
}
