#include "debug.hpp"

#include "asm.hpp"
#include "terminal.hpp"
#include "cstring.hpp"
#include "memory.hpp"

namespace debug
{
    void memusage(void)
    {
        term::write("Memory used: 0x", false);
        term::write(cstr::convert(mem::used(), 16), true);
        term::write(" | empty: 0x", false);
        term::writeline(cstr::convert(mem::empty(), 16), true);
    }

    void panic(const char* const str)
    {
        term::setcolor(term::vga_entry_color(term::vga_color::VGA_COLOR_RED, term::vga_color::VGA_COLOR_LIGHT_GREEN));
        term::clear();
        term::writeline(str, false);
        hlt();
    }

    void panic(void)
    {
        panic(cstr::center("Fatal Error: Kernel Panic", term::VGA_WIDTH, term::VGA_HEIGHT));
    }
}
