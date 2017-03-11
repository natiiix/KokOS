#include "terminal.hpp"

namespace term
{
	size_t row;
	size_t column;
	uint8_t color;
	uint16_t* buffer;

	void initialize(void)
	{
		row = 0;
		column = 0;
		color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
		buffer = (uint16_t*) 0xB8000;
		for (size_t y = 0; y < VGA_HEIGHT; y++)
		{
			for (size_t x = 0; x < VGA_WIDTH; x++)
			{
				const size_t index = y * VGA_WIDTH + x;
				buffer[index] = vga_entry(' ', color);
			}
		}
	}
	
	void setcolor(uint8_t color)
	{
		color = color;
	}
	
	void putentryat(char c, uint8_t color, size_t x, size_t y)
	{
		const size_t index = y * VGA_WIDTH + x;
		buffer[index] = vga_entry(c, color);
	}
	
	void putchar(char c)
	{
		if (c == '\n')
		{		
			// scroll screen on screen overflow
			if (++row == VGA_HEIGHT)
			{			
				// scroll rows up
				for (size_t y = 0; y < VGA_HEIGHT - 1; y++)
				{
					for (size_t x = 0; x < VGA_WIDTH; x++)
					{
						const size_t index = y * VGA_WIDTH + x;
						const size_t source = index + VGA_WIDTH;
						buffer[index] = buffer[source];
					}
				}
				
				row = VGA_HEIGHT - 1;
				
				// clear the last row
				for (size_t x = 0; x < VGA_WIDTH; x++)
				{
					const size_t index = row * VGA_WIDTH + x;
					buffer[index] = vga_entry(' ', color);
				}
			}
			
			column = 0;
			return;
		}

		putentryat(c, color, column, row);
		
		if (++column == VGA_WIDTH)
		{
			column = 0;
			if (++row == VGA_HEIGHT)
				row = 0;
		}
	}

	void newline(void)
	{
		putchar('\n');
	}
	
	void write(const char* data)
	{
		size_t i = 0;
		while (data[i])
			putchar(data[i++]);
	}

	void writeline(const char* data)
	{
		write(data);

		// Prevent empty line after an input that fills the whole row
		// Empty input string (first character is '\0') implies the intent to create an empty line
		if (column > 0 || !data[0])
			newline();
	}

	void setcursor(int currow, int curcol)
	{
		unsigned short position = (currow * VGA_WIDTH) + curcol;

		// cursor LOW port to vga INDEX register
		outb(0x3D4, 0x0F);
		outb(0x3D5, (unsigned char)(position & 0xFF));
		// cursor HIGH port to vga INDEX register
		outb(0x3D4, 0x0E);
		outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
	}
}
