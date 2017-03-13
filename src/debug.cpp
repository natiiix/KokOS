#include "debug.hpp"

namespace debug
{
    void memusage(void)
    {
        term::write("Memory used: 0x");
        term::memdump(str::convert(mem::used(), 16), false);
        term::write(" | empty: 0x");
        term::memdump(str::convert(mem::empty(), 16), true);
    }
}
