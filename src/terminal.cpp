#include "terminal.hpp"

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

void terminal_initialize(void)
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++)
	{
		for (size_t x = 0; x < VGA_WIDTH; x++)
		{
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}
 
void terminal_setcolor(uint8_t color)
{
	terminal_color = color;
}
 
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}
 
void terminal_putchar(char c)
{
	if (c == '\n')
	{		
		// scroll screen on screen overflow
		if (++terminal_row == VGA_HEIGHT)
		{			
			// scroll rows up
			for (size_t y = 0; y < VGA_HEIGHT - 1; y++)
			{
				for (size_t x = 0; x < VGA_WIDTH; x++)
				{
					const size_t index = y * VGA_WIDTH + x;
					const size_t source = index + VGA_WIDTH;
					terminal_buffer[index] = terminal_buffer[source];
				}
			}
			
			terminal_row = VGA_HEIGHT - 1;
			
			// clear the last row
			for (size_t x = 0; x < VGA_WIDTH; x++)
			{
				const size_t index = terminal_row * VGA_WIDTH + x;
				terminal_buffer[index] = vga_entry(' ', terminal_color);
			}
		}
		
		terminal_column = 0;
		return;
	}

	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	
	if (++terminal_column == VGA_WIDTH)
	{
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	}
}

void terminal_newline(void)
{
	terminal_putchar('\n');
}
 
void terminal_write(const char* data)
{
    size_t i = 0;
    while (data[i])
		terminal_putchar(data[i++]);
}

void terminal_writeline(const char* data)
{
    terminal_write(data);

	// Prevent empty line after an input that fills the whole row
	// Empty input string (first character is '\0') implies the intent to create an empty line
	if (terminal_column > 0 || !data[0])
		terminal_newline();
}

void terminal_setcursor(int row, int col)
{
	unsigned short position = (row * VGA_WIDTH) + col;

	// cursor LOW port to vga INDEX register
	outb(0x3D4, 0x0F);
	outb(0x3D5, (unsigned char)(position & 0xFF));
	// cursor HIGH port to vga INDEX register
	outb(0x3D4, 0x0E);
	outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}
