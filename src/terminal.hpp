#pragma once

#include "stdtypes.hpp"
#include "asm.hpp"

namespace term
{
	// Screen width in text mode in characters
	static const size_t VGA_WIDTH = 80;
	// Screen height in text mode in characters
	static const size_t VGA_HEIGHT = 25;

	extern size_t row;
	extern size_t column;
	extern uint8_t color;
	extern uint16_t* buffer;

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
	static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
	{ return fg | bg << 4; }
	// Converts a character and VGA color into a VGA entry
	static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
	{ return (uint16_t) uc | (uint16_t) color << 8; }
	// Initializes terminal
	void initialize(void); 
	// Sets the terminal color
	void setcolor(uint8_t color); 
	// Writes a character in specified color at specified coordinates in the buffer
	void putentryat(char c, uint8_t color, size_t x, size_t y);
	// Writes a character into the terminal buffer
	void putchar(char c);
	// Creates a new line 
	void newline(void);
	// Writes a string into the terminal buffer
	void write(const char* data);
	// Writes a string into the terminal buffer and ends the line
	void writeline(const char* data);
	// Moves the terminal cursor to a specified position
	void setcursor(int currow, int curcol);
}
