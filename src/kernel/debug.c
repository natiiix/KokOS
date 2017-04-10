#include <drivers/io/terminal.h>
#include <c/string.h>
#include <drivers/memory.h>

// The code inside debug_*() functions is only executed in DEBUG mode

void debug_memusage(void)
{
    #ifdef DEBUG

    term_write("Memory used: 0x", false);
    term_write(tostr(mem_used(), 16), true);
    term_write(" | empty: 0x", false);
    term_writeline(tostr(mem_empty(), 16), true);

    #endif
}

void debug_print(const char* const str)
{
    #ifdef DEBUG

    term_write("DEBUG: ", false);
    term_writeline(str, false);

    #endif
}

void debug_pause(void)
{
    #ifdef DEBUG

    term_pause();

    #endif
}
