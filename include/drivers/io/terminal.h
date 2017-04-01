#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "terminal_global.h"

// Converts foreground and background colors into a single VGA color used by VGA entries
static inline uint8_t vga_entry_color(const enum VGA_COLOR fg, const enum VGA_COLOR bg)
{ return fg | bg << 4; }
// Converts a character and VGA color into a VGA entry
static inline uint16_t vga_entry(const unsigned char uc, const uint8_t color)
{ return (uint16_t) uc | (uint16_t) color << 8; }

// Clears the terminal
void term_clear(void);
// Sets the terminal color
void term_setcolor(const uint8_t color);
// Breaks the line
void term_breakline(void);
// Writes a string into the terminal buffer
// dipose - should the memory used by str be freed after writing
void term_write(const char* const str, const bool dispose);
void term_write_convert(const size_t input, const size_t base);
// Extension of term_write() which breaks the line after writing
void term_writeline(const char* const str, const bool dispose);
void term_writeline_convert(const size_t input, const size_t base);
// Moves the terminal cursor to a specified position
void term_setcursor(const size_t col, const size_t row);
// Reads a string from the input buffer into memory and returns a pointer to it
char* term_readline(void);
// Writes "Press ENTER to continue..." and waits for ENTER to be pressed
void term_pause(void);

void term_writeat(const char* const str, const size_t col, const size_t row);
void term_setcolorfg(const enum VGA_COLOR color);
void term_setcolorbg(const enum VGA_COLOR color);
void term_setactive(const size_t col, const size_t row);
size_t term_getcol(void);
size_t term_getrow(void);
void term_newline(void);
