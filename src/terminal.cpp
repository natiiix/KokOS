#include "terminal.hpp"

namespace term
{
	size_t m_row;
	size_t m_column;
	uint8_t m_color;
	uint16_t* m_buffer;
	bool m_linebroken;

	void init(void)
	{
		m_row = 0;
		m_column = 0;
		m_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
		m_buffer = (uint16_t*) 0xB8000;

		for (size_t y = 0; y < VGA_HEIGHT; y++)
		{
			for (size_t x = 0; x < VGA_WIDTH; x++)
			{
				const size_t index = y * VGA_WIDTH + x;
				m_buffer[index] = vga_entry(' ', m_color);
			}
		}

		m_linebroken = false;

        keybd::init();
	}
	
	void setcolor(const uint8_t color)
	{
		m_color = color;
	}
	
	// Writes a character in specified color at specified coordinates in the buffer
	void putentryat(const char c, const uint8_t color, const size_t x, const size_t y)
	{
		const size_t index = y * VGA_WIDTH + x;
		m_buffer[index] = vga_entry(c, color);
	}

	// Creates a new line
	void newline(void)
	{
		// scroll screen on screen overflow
		if (++m_row == VGA_HEIGHT - 1)
		{	
			m_row = VGA_HEIGHT - 2;

			// scroll rows up
			for (size_t y = 0; y < VGA_HEIGHT - 2; y++)
			{
				for (size_t x = 0; x < VGA_WIDTH; x++)
				{
					const size_t index = y * VGA_WIDTH + x;
					const size_t source = index + VGA_WIDTH;
					m_buffer[index] = m_buffer[source];
				}
			}
			
			// clear the last row
			for (size_t x = 0; x < VGA_WIDTH; x++)
			{
				const size_t index = m_row * VGA_WIDTH + x;
				m_buffer[index] = vga_entry(' ', m_color);
			}
		}
		
		m_column = 0;
		m_linebroken = false;
	}

	void breakline(void)
	{
		if (m_linebroken)
			newline();

		m_linebroken = true;
	}
	
	// Writes a character into the terminal buffer
	void putchar(const char c)
	{
		if (m_linebroken)
		{
			newline();
		}

		if (c == '\n')
		{
			breakline();
			return;
		}

		putentryat(c, m_color, m_column, m_row);
		
		if (++m_column == VGA_WIDTH)
		{
			m_column = 0;
			breakline();
		}
	}
	
	void write(const char* const str)
	{
		for (size_t i = 0; str[i]; i++)
		{
			putchar(str[i]);
		}
	}

	void writeline(const char* const str)
	{
		write(str);

		// Prevent empty line after an input that fills the whole row
		// Empty input string (first character is '\0') implies the intent to create an empty line
		if (m_column > 0 || !str[0])
			breakline();
	}

	void memdump(const void* const memstr, const bool linebreak)
	{
		if (linebreak)
			writeline((char*)memstr);
		else
			write((char*)memstr);

		mem::free(memstr);
	}

	void setcursor(const int row, const int col)
	{
		unsigned short position = (row * VGA_WIDTH) + col;

		// cursor LOW port to vga INDEX register
		outb(0x3D4, 0x0F);
		outb(0x3D5, (unsigned char)(position & 0xFF));
		// cursor HIGH port to vga INDEX register
		outb(0x3D4, 0x0E);
		outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
	}

	void updateinputrow(const char* const inbuff)
    {
        size_t length = str::len(inbuff);

		const size_t prefixlen = 1;
		const char prefix[prefixlen] = { '>' };

		// prefix, input and terminal cursor
		size_t totallen = prefixlen + length + 1;
		size_t startchar = 0;
		if (totallen > VGA_WIDTH)
			startchar = totallen - VGA_WIDTH;

		size_t renderlen = length - startchar;
        
		for (size_t i = 0; i < VGA_WIDTH; i++)
        {
			char c = 0;

			if (i < prefixlen)
				c = prefix[i];
			else if (i < prefixlen + renderlen)
				c = inbuff[i - prefixlen + startchar];
			else
				c = ' ';

			putentryat(c, term::m_color, i, VGA_HEIGHT - 1);
        }

		term::setcursor(term::VGA_HEIGHT - 1, prefixlen + renderlen);
    }

	char* readline(void)
	{
		input::init();

		char* inputbuffer = &input::buffer[0];

		while (input::handlekey())
		{
			updateinputrow(inputbuffer);
		}

		return (char*)mem::copy((void*)inputbuffer);
	}
}
