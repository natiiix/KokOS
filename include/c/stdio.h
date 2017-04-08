#pragma once

#include <stddef.h>
#include <stdint.h>
#include <drivers/io/terminal_global.h>
#include <drivers/io/keyboard_global.h>

#if defined(__cplusplus)
extern "C"
{
#endif

void clear(void);
void setcolor(const enum VGA_COLOR fg, const enum VGA_COLOR bg);
void setfg(const enum VGA_COLOR color);
void setbg(const enum VGA_COLOR color);
void setcursor(const size_t col, const size_t row);
void setactive(const size_t col, const size_t row);
void print(const char* const str);
void printat(const char* const str, const size_t col, const size_t row);
void printint(const size_t i);
void printbin(const size_t i);
void printhex(const size_t i);
size_t getcol(void);
size_t getrow(void);
void newline(void);
void pause(void);
struct keyevent readKeyEvent(void);

#if defined(__cplusplus)
}
#endif
