#include <assembly.h>
#include <drivers/io/terminal.h>
#include <c/string.h>

void kernel_panic(const char* const str)
{
    // Choice of colors inspired by old Windows BSOD
    static const enum VGA_COLOR panicColorBG = VGA_COLOR_BLUE;
    static const enum VGA_COLOR panicColorFG = VGA_COLOR_WHITE;

    static const char strHeader[] = "Kernel Panic\n\n\n\n";

    term_setcolor(vga_entry_color(panicColorFG, panicColorBG));
    term_clear();

    // The memory should normally be freed, but since the kernel has panicked
    // it shouldn't really matter since it's not expected to recover
    term_write(strcenter(strjoin(&strHeader[0], str)), false);

    while (true)
    {
        hlt();
    }
}
