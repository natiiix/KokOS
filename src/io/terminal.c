#include <io/terminal.h>
#include <drivers/memory.h>
#include <c/string.h>
#include <assembly.h>
#include <drivers/io/keyboard.h>

uint16_t* vgaBuffer;
uint8_t activeColor;
size_t activeRow;
size_t activeColumn;
bool lineBroken;

void term_init(void)
{
    vgaBuffer = (uint16_t*)0xC07FF000;
	activeColor = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

	term_clear();
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
void _newline(void)
{
	// scroll screen on screen overflow
	if (++activeRow == VGA_HEIGHT - 1)
	{	
		activeRow = VGA_HEIGHT - 2;

		// scroll rows up
		for (size_t y = 0; y < VGA_HEIGHT - 2; y++)
		{
			for (size_t x = 0; x < VGA_WIDTH; x++)
			{
				const size_t index = y * VGA_WIDTH + x;
				const size_t source = index + VGA_WIDTH;
				vgaBuffer[index] = vgaBuffer[source];
			}
		}
		
		// clear the last row
		for (size_t x = 0; x < VGA_WIDTH; x++)
		{
			const size_t index = activeRow * VGA_WIDTH + x;
			vgaBuffer[index] = vga_entry(' ', activeColor);
		}
	}
	
	activeColumn = 0;
	lineBroken = false;
}

void term_breakline(void)
{
	if (lineBroken)
	{
		_newline();
	}

	lineBroken = true;
}

// Writes a character into the terminal buffer
void _putchar(const char c)
{
	if (lineBroken)
	{
		_newline();
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
	
	term_setcursor(activeRow, activeColumn);

	if (dispose)
	{
		free(str);
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

void term_setcursor(const int row, const int col)
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
	size_t length = strlen(inbuff);

	#define PREFIX_LENGTH 1
	const char prefix[PREFIX_LENGTH] = { '>' };

	// prefix, input and terminal cursor
	size_t totallen = PREFIX_LENGTH + length + 1;
	size_t startchar = 0;
	if (totallen > VGA_WIDTH)
		startchar = totallen - VGA_WIDTH;

	size_t renderlen = length - startchar;
	
	for (size_t i = 0; i < VGA_WIDTH; i++)
	{
		char c = 0;

		if (i < PREFIX_LENGTH)
			c = prefix[i];
		else if (i < PREFIX_LENGTH + renderlen)
			c = inbuff[i - PREFIX_LENGTH + startchar];
		else
			c = ' ';

		_putentryat(c, activeColor, i, VGA_HEIGHT - 1);
	}

	term_setcursor(VGA_HEIGHT - 1, PREFIX_LENGTH + renderlen);
}

void _clearinputrow(void)
{
	for (size_t i = 0; i < VGA_WIDTH; i++)
	{
		_putentryat(' ', activeColor, i, VGA_HEIGHT - 1);
	}
}

char* term_readline(void)
{
	char* inputbuffer = (char*)alloc(1024);
	size_t bufferptr = 0;
	inputbuffer[0] = '\0';

	while (true)
	{
		uint8_t scancode = keybd_readkey();
		uint8_t keycode = scancode & 0b01111111;
		bool keystate = (scancode == keycode);

		if (keyPressed[keycode] != keystate)
		{
			keyPressed[keycode] = keystate;
			
			if (keystate)
			{
				char inchar = scancodeToChar(keycode, keyPressed[KEY_SHIFT_LEFT] || keyPressed[KEY_SHIFT_RIGHT]);
				
				if (inchar > 0)
				{
					inputbuffer[bufferptr++] = inchar;
					inputbuffer[bufferptr] = '\0';
				}
				else if (keycode == KEY_ENTER)
				{
					_clearinputrow();
					return inputbuffer;
				}
				else if (keycode == KEY_BACKSPACE)
				{
					// Make sure the cursor doesn't get past the beginning of the input row
					if (bufferptr > 0)
						inputbuffer[--bufferptr] = '\0';
				}    
				else if (keycode == KEY_ESCAPE)
				{
					inputbuffer[bufferptr = 0] = '\0';
				}
			}
		}

		_updateinputrow(inputbuffer);
	}
}

void term_pause(void)
{
	term_write("Press ENTER to continue...", false);

	while (true)
	{
		uint8_t scancode = keybd_readkey();
		uint8_t keycode = scancode & 0b01111111;
		bool keystate = (scancode == keycode);

		if (keyPressed[keycode] != keystate)
		{
			keyPressed[keycode] = keystate;
			
			if (keycode == KEY_ENTER && keystate)
			{
				break;
			}
		}
	}

	term_breakline();
}
