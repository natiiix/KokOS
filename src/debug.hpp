#pragma once

#include "asm.hpp"
#include "terminal.hpp"
#include "cstring.hpp"
#include "memory.hpp"

namespace debug
{
    void memusage(void);
    void panic(const char* const str);
    void panic(void);
}
