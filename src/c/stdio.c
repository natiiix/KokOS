#include <drivers/io/terminal.h>
#include <drivers/io/keyboard.h>
#include <kernel.h>

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

void printint(const size_t i)
{
    term_write_convert(i, 10);
}

void printbin(const size_t i)
{
    term_write_convert(i, 2);
}

void printhex(const size_t i)
{
    term_write_convert(i, 16);
}

size_t getcol(void)
{
    return term_getcol();
}

size_t getrow(void)
{
    return term_getrow();
}

void newline(void)
{
    term_newline();
}

void pause(void)
{
    term_pause();
}

struct keyevent readKeyEvent(void)
{
    return keybd_read();
}
