#include <assembly.h>
#include <io/terminal.h>
#include <c/string.h>

void kernel_panic(const char* const str)
{
    term_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLUE));
    term_clear();
    term_writeline(str, false);

    hlt();
}

void kernel_panic_default(void)
{
    kernel_panic(strcenter("Fatal Error: Kernel Panic", VGA_WIDTH, VGA_HEIGHT));
}
