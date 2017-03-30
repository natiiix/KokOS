#pragma once

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

// Screen width in text mode in characters
static const size_t VGA_WIDTH = 80;
// Screen height in text mode in characters
static const size_t VGA_HEIGHT = 25;

// Hardware text mode color constants
enum VGA_COLOR : uint8_t
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

void clear(void);
void setcolor(const enum VGA_COLOR fg, const enum VGA_COLOR bg);
void setfg(const enum VGA_COLOR color);
void setbg(const enum VGA_COLOR color);
void setcursor(const size_t col, const size_t row);
void setactive(const size_t col, const size_t row);
void print(const char* const str);
void printat(const char* const str, const size_t col, const size_t row);
char* scan(void);

#if defined(__cplusplus)
}
#endif
