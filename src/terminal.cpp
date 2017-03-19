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
		m_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
		m_buffer = (uint16_t*)0xC07FF000;

		clear();

        keybd::init();
	}

	void clear(void)
	{
		for (size_t y = 0; y < VGA_HEIGHT; y++)
		{
			for (size_t x = 0; x < VGA_WIDTH; x++)
			{
				const size_t index = y * VGA_WIDTH + x;
				m_buffer[index] = vga_entry(' ', m_color);
			}
		}

		m_row = 0;
		m_column = 0;
		m_linebroken = false;
	}
	
	void setcolor(const uint8_t color)
	{
		m_color = color;
	}
	
	// Writes a character in specified color at specified coordinates in the buffer
	void _putentryat(const char c, const uint8_t color, const size_t x, const size_t y)
	{
		const size_t index = y * VGA_WIDTH + x;
		m_buffer[index] = vga_entry(c, color);
	}

	// Creates a new line
	void _newline(void)
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
			_newline();

		m_linebroken = true;
	}
	
	// Writes a character into the terminal buffer
	void _putchar(const char c)
	{
		if (m_linebroken)
		{
			_newline();
		}

		if (c == '\n')
		{
			breakline();
			return;
		}

		_putentryat(c, m_color, m_column, m_row);
		
		if (++m_column == VGA_WIDTH)
		{
			m_column = 0;
			breakline();
		}
	}
	
	void write(const char* const str, const bool dispose)
	{
		for (size_t i = 0; str[i]; i++)
		{
			_putchar(str[i]);
		}
		
		setcursor(m_row, m_column);

		if (dispose)
		{
			delete str;
		}
	}

	void write(const size_t input, const size_t base)
	{
		char* str = cstr::convert(input, base);
		write(str, true);
	}

	// Local extension of breakline() used by writeline() to avoid undesired empty lines
	void _breakline_writeline(void)
	{
		// Prevent empty line after an input that fills the whole row (and naturally breaks line by itself)
		if (m_column > 0)
			breakline();
	}

	void writeline(const char* const str, const bool dispose)
	{
		// Empty input string (first character is '\0') implies the intent to create an empty line
		if (str[0] == '\0')
		{
			breakline();
			return;
		}

		write(str, dispose);
		_breakline_writeline();
	}

	void writeline(const size_t input, const size_t base)
	{
		write(input, base);
		_breakline_writeline();
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

	void _updateinputrow(const char* const inbuff)
    {
        size_t length = cstr::len(inbuff);

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

			_putentryat(c, term::m_color, i, VGA_HEIGHT - 1);
        }

		term::setcursor(term::VGA_HEIGHT - 1, prefixlen + renderlen);
    }

	char* readline(void)
	{
		input::init();

		char* inputbuffer = &input::buffer[0];

		while (input::handlekey())
		{
			_updateinputrow(inputbuffer);
		}

		return (char*)mem::copy(inputbuffer);
	}

	void pause(void)
	{
		write("Press ENTER to continue...", false);

		while (true)
		{
			uint8_t scancode = keybd::readkey();
			uint8_t keycode = scancode & 0b01111111;
			bool keystate = (scancode == keycode);

			if (keybd::keypressed[keycode] != keystate)
			{
				keybd::keypressed[keycode] = keystate;
				
				if (keycode == keybd::KEY_ENTER && keystate)
				{
					break;
				}
			}
		}

		breakline();
	}
}
