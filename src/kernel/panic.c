#include <assembly.h>
#include <drivers/io/terminal.h>
#include <c/string.h>

void kernel_panic(const char* const str)
{
    // Choice of colors inspired by old Windows BSOD
    static const enum VGA_COLOR colorBackground = VGA_COLOR_BLUE;
    static const enum VGA_COLOR colorForeground = VGA_COLOR_WHITE;

    static const char strHeader[] = "Kernel Panic\n\n\n\n";

    term_setcolor(vga_entry_color(colorForeground, colorBackground));
    term_clear();

    // The memory should normally be freed, but since the kernel has panicked
    // it shouldn't really matter since it's not expected to recover
    term_writeline(strcenter(strjoin(&strHeader[0], str)), false);

    hlt();
}
