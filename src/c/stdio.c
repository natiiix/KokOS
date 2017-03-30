#include <io/terminal.h>

void clear(void)
{
    term_clear();
}

void setcolor(const enum VGA_COLOR fg, const enum VGA_COLOR bg)
{
    term_setcolor(vga_entry_color(fg, bg));
}

void setbg(const enum VGA_COLOR color)
{
    term_setcolorbg(color);
}

void setfg(const enum VGA_COLOR color)
{
    term_setcolorfg(color);
}

void setcursor(const size_t col, const size_t row)
{
    term_setcursor(col, row);
}

void setactive(const size_t col, const size_t row)
{
    term_setactive(col, row);
}

void print(const char* const str)
{
    term_write(str, false);
}

void printat(const char* const str, const size_t col, const size_t row)
{
    term_writeat(str, col, row);
}

char* scan(void)
{
    return term_readline();
}
