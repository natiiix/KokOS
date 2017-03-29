#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Screen width in text mode in characters
static const size_t VGA_WIDTH = 80;
// Screen height in text mode in characters
static const size_t VGA_HEIGHT = 25;

// Hardware text mode color constants
enum VGA_COLOR
{
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

// Converts foreground and background colors into a single VGA color used by VGA entries
static inline uint8_t vga_entry_color(const enum VGA_COLOR fg, const enum VGA_COLOR bg)
{ return fg | bg << 4; }
// Converts a character and VGA color into a VGA entry
static inline uint16_t vga_entry(const unsigned char uc, const uint8_t color)
{ return (uint16_t) uc | (uint16_t) color << 8; }

// Initializes terminal
void term_init(void);
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
void term_setcursor(const int row, const int col);
// Reads a string from the input buffer into memory and returns a pointer to it
char* term_readline(void);
// Writes "Press ENTER to continue..." and waits for ENTER to be pressed
void term_pause(void);
