#include <drivers/io/terminal.h>
#include <drivers/memory.h>
#include <c/string.h>
#include <assembly.h>
#include <drivers/io/keyboard.h>

uint16_t* vgaBuffer;
uint8_t activeColor;
size_t activeRow;
size_t activeColumn;
bool lineBroken;

void term_enable_cursor(void)
{
	outb(0x3D4, 0x0A);		// set the cursor start scanline to 13(0x0d) and enable cursor visibility
	outb(0x3D5, 0x0D);

	outb(0x3D4, 0x0B);
	char scanend = inb(0x3D5) & ~(0x1F);
					// get byte that has the current scanline end (lower 5 bits) and set them to 0
					// we want to keep the upper 3 bits unchanged
	scanend |= 0x0e;		// set the end scanline to 14(0x0e).
	outb(0x3D4, 0x0B);
	outb(0x3D5, scanend);
}

void term_init(void)
{
	vgaBuffer = (uint16_t*)0xB8000;
	activeColor = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

	term_clear();
	term_enable_cursor();
	term_writeline("Terminal initialized.", false);
}

void term_clear(void)
{
    size_t index = 0;

	for (size_t y = 0; y < VGA_HEIGHT; y++)
	{
		for (size_t x = 0; x < VGA_WIDTH; x++)
		{
			vgaBuffer[index++] = vga_entry(' ', activeColor);
		}
	}

	activeRow = 0;
	activeColumn = 0;
	lineBroken = false;
}

void term_setcolor(const uint8_t color)
{
	activeColor = color;
}

// Writes a character in specified color at specified coordinates in the buffer
void _putentryat(const char c, const uint8_t color, const size_t x, const size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	vgaBuffer[index] = vga_entry(c, color);
}

// Creates a new line
void term_newline(void)
{
	// scroll screen on screen overflow
	if (++activeRow == VGA_HEIGHT)
	{	
		activeRow = VGA_HEIGHT - 1;

		// scroll rows up
		size_t index = 0;		
		for (size_t y = 0; y < VGA_HEIGHT - 1; y++)
		{
			for (size_t x = 0; x < VGA_WIDTH; x++)
			{
				vgaBuffer[index] = vgaBuffer[index + VGA_WIDTH];
				index++;
			}
		}
		
		// clear the last row
		const size_t rowBegin = activeRow * VGA_WIDTH;
		for (size_t x = 0; x < VGA_WIDTH; x++)
		{
			vgaBuffer[rowBegin + x] = vga_entry(' ', activeColor);
		}
	}
	
	activeColumn = 0;
	lineBroken = false;
}

void term_breakline(void)
{
	if (lineBroken)
	{
		term_newline();
	}

	lineBroken = true;
}

// Writes a character into the terminal buffer
void _putchar(const char c)
{
	if (lineBroken)
	{
		term_newline();
	}

	if (c == '\n')
	{
		term_breakline();
		return;
	}

	_putentryat(c, activeColor, activeColumn, activeRow);
	
	if (++activeColumn == VGA_WIDTH)
	{
		activeColumn = 0;
		term_breakline();
	}
}

void term_write(const char* const str, const bool dispose)
{
	for (size_t i = 0; str[i]; i++)
	{
		_putchar(str[i]);
	}
	
	term_setcursor(activeColumn, activeRow);

	if (dispose)
	{
		mem_free(str);
	}
}

void term_write_convert(const size_t input, const size_t base)
{
	char* str = tostr(input, base);
	term_write(str, true);
}

// Local extension of breakline() used by writeline() to avoid undesired empty lines
void _breakline_writeline(void)
{
	// Prevent empty line after an input that fills the whole row (and naturally breaks line by itself)
	if (activeColumn > 0)
		term_breakline();
}

void term_writeline(const char* const str, const bool dispose)
{
	// Empty input string (first character is '\0') implies the intent to create an empty line
	if (str[0] == '\0')
	{
		term_breakline();
		return;
	}

	term_write(str, dispose);
	_breakline_writeline();
}

void term_writeline_convert(const size_t input, const size_t base)
{
	term_write_convert(input, base);
	_breakline_writeline();
}

void term_setcursor(const size_t col, const size_t row)
{
	uint16_t position = (row * VGA_WIDTH) + col;

	// cursor LOW port to vga INDEX register
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)position);
	// cursor HIGH port to vga INDEX register
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)(position >> 8));
}

void _updateinputrow(const char* const inbuff)
{
	size_t length = strlen(inbuff);

	#define PREFIX_LENGTH 1
	const char prefix[PREFIX_LENGTH] = { '>' };

	// prefix, input and terminal cursor
	size_t totallen = PREFIX_LENGTH + length + 1;
	size_t startchar = 0;

	size_t spaceAvailable = VGA_WIDTH - activeColumn;

	if (totallen > spaceAvailable)
	{
		startchar = totallen - spaceAvailable;
	}

	size_t renderlen = length - startchar;
	
	for (size_t i = 0; i < spaceAvailable; i++)
	{
		char c = 0;

		if (i < PREFIX_LENGTH)
			c = prefix[i];
		else if (i < PREFIX_LENGTH + renderlen)
			c = inbuff[i - PREFIX_LENGTH + startchar];
		else
			c = ' ';

		_putentryat(c, activeColor, activeColumn + i, activeRow);
	}

	term_setcursor(activeColumn + PREFIX_LENGTH + renderlen, activeRow);
}

void _clearinputrow(void)
{
	for (size_t i = 0; i < VGA_WIDTH; i++)
	{
		_putentryat(' ', activeColor, i, activeRow);
	}
}

void term_pause(void)
{
	term_write("Press ENTER to continue...", false);

	while (true)
	{
		struct keyevent ke = keybd_read();

		if (ke.scancode == KEY_ENTER && ke.state == KEY_DOWN)
		{
			break;
		}
	}

	term_breakline();
}

void term_writeat(const char* const str, const size_t col, const size_t row)
{
	size_t spaceAvailable = VGA_WIDTH - col;

	for (size_t i = 0; i < spaceAvailable && str[i]; i++)
	{
		_putentryat(str[i], activeColor, col + i, row);
	}
}

void term_setcolorfg(const enum VGA_COLOR color)
{
	activeColor &= 0xF0;
	activeColor |= color;
}

void term_setcolorbg(const enum VGA_COLOR color)
{
	activeColor &= 0x0F;
	activeColor |= (color << 4);
}

void term_setactive(const size_t col, const size_t row)
{
	activeRow = row;
	activeColumn = col;
}

size_t term_getcol(void)
{
	return activeColumn;
}

size_t term_getrow(void)
{
	return activeRow;
}
