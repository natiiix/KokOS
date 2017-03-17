#include "debug.hpp"

namespace debug
{
    void memusage(void)
    {
        term::write("Memory used: 0x", false);
        term::write(cstr::convert(mem::used(), 16), true);
        term::write(" | empty: 0x", false);
        term::writeline(cstr::convert(mem::empty(), 16), true);
    }
}
