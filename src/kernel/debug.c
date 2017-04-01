#include <assembly.h>
#include <drivers/io/terminal.h>
#include <c/string.h>
#include <drivers/memory.h>

void debug_memusage(void)
{
    term_write("Memory used: 0x", false);
    term_write(tostr(mem_used(), 16), true);
    term_write(" | empty: 0x", false);
    term_writeline(tostr(mem_empty(), 16), true);
}
