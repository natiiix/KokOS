#pragma once

#include <stddef.h>
#include <stdint.h>

#include "asm.hpp"
#include "input.hpp"
#include "cstring.hpp"
#include "keyboard.hpp"
#include "memory.hpp"

namespace term
{
	// Screen width in text mode in characters
	static const size_t VGA_WIDTH = 80;
	// Screen height in text mode in characters
	static const size_t VGA_HEIGHT = 25;

	// Hardware text mode color constants
	enum vga_color
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
	static inline uint8_t vga_entry_color(const enum vga_color fg, const enum vga_color bg)
	{ return fg | bg << 4; }
	// Converts a character and VGA color into a VGA entry
	static inline uint16_t vga_entry(const unsigned char uc, const uint8_t color)
	{ return (uint16_t) uc | (uint16_t) color << 8; }
	// Initializes terminal
	void init(void);
	// Clears the terminal
	void clear(void);
	// Sets the terminal color
	void setcolor(const uint8_t color);
	// Breaks the line
	void breakline(void);
	// Writes a string into the terminal buffer
	// dipose - should the memory used by str be freed after writing
	void write(const char* const str, const bool dispose = false);
	void write(const size_t input, const size_t base = 10);
	// Extension of write() which breaks the line after writing
	void writeline(const char* const str, const bool dispose = false);
	void writeline(const size_t input, const size_t base = 10);
	// Moves the terminal cursor to a specified position
	void setcursor(const int row, const int col);
	// Reads a string from the input buffer into memory and returns a pointer to it
	char* readline(void);
	// Writes "Press ENTER to continue..." and waits for ENTER to be pressed
	void pause(void);
}
