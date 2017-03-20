#pragma once

#include <stddef.h>
#include <stdint.h>

namespace cstr
{
    size_t len(const char* const str);
    bool cmp(const char* const str1, const char* const str2);
    char* convert(const size_t input, const size_t base = 10);
    size_t parse(const char* const str, const size_t base = 10);
    char* center(const char* const str, const size_t width, const size_t height);
}
