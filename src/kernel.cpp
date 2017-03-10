/* Surely you will remove the processor conditionals and this comment
   appropriately depending on whether or not you use C++. */
#if !defined(__cplusplus)
#include <stdbool.h> /* C doesn't have booleans by default. */
#endif
#include <stddef.h>
#include <stdint.h>
 
/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

/* Hardware text mode color constants. */
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
 
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}
 
static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
	return (uint16_t) uc | (uint16_t) color << 8;
}
 
size_t strlen(const char* str)
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
 
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
 
void terminal_write(const char* data, size_t size)
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}
 
void terminal_writestring(const char* data)
{
	terminal_write(data, strlen(data));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
    /* There's an outb %al, $imm8  encoding, for compile-time constant port numbers that fit in 8b.  (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}

uint8_t getScancode()
{
	uint8_t c = 0;
	
	do
	{
		if(inb(0x60) != c)
		{
			c = inb(0x60);
			if(c > 0)
			return c;
		}
	}
	while (1);
}

const char scancodeASCII[] =
{
0,
0, 	// ESC
49,	// 1
50,
51,
52,
53,
54,
55,
56,
57, // 9
48,	// 0
45,	// -
61,	// =
0,
0,
113,// q
119,
101,
114,
116,
121,
117,
105,
111,
112,// p
91,	// [
93,	// ]
0,
0,
97,	// a
115,
100,
102,
103,
104,
106,
107,
108,// l
59,	// ;
39,	// '
96,	// `
0,
92,	// BACKSLASH
122,// z
120,
99,
118,
98,
110,
109,// m
44,	// ,
46,	// .
47,	// /
0,
42,	// *
0,
32,	// SPACE
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0
};

char scancodeToChar(uint8_t sc)
{
	return scancodeASCII[sc];
}

/* void update_cursor(int row, int col)
* by Dark Fiber
*/
void update_cursor(int row, int col)
{
	unsigned short position = (row * VGA_WIDTH) + col;

	// cursor LOW port to vga INDEX register
	outb(0x3D4, 0x0F);
	outb(0x3D5, (unsigned char)(position & 0xFF));
	// cursor HIGH port to vga INDEX register
	outb(0x3D4, 0x0E);
	outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

void updateInputRow(const char* inbuff)
{
	bool endReached = false;
	for (size_t i = 0; i < VGA_WIDTH; i++)
	{
		if (inbuff[i] == '\0')
			endReached = true;
		
		if (endReached)
			terminal_putentryat(' ', terminal_color, i, VGA_HEIGHT - 1);
		else
			terminal_putentryat(inbuff[i], terminal_color, i, VGA_HEIGHT - 1);
	}
}

void flushInput(char* const inbuff, size_t* const inbuffptr)
{
	terminal_writestring(&inbuff[0]); // flush the buffer
	if (*inbuffptr < VGA_WIDTH - 1)
		terminal_writestring("\n"); // new line
	*inbuffptr = 0; // reset the buffer pointer
	inbuff[0] = '\0'; // clear the buffer
}
 
#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif
void kernel_main(void)
{
	/* Initialize terminal interface */
	terminal_initialize();
	
	bool keypressed[128];
	for (size_t i = 0; i < 128; i++)
		keypressed[i] = false;
	
	char inputbuffer[VGA_WIDTH];
	size_t inputbufferptr = 0;
	inputbuffer[0] = '\0';
	
	while (true)
	{		
		uint8_t c = getScancode();
		uint8_t keycode = c & 0b01111111;
		bool keystate = (c == keycode);
		if (keypressed[keycode] != keystate)
		{
			keypressed[keycode] = keystate;
			
			if (keystate)
			{
				char inchar = scancodeToChar(keycode);
				
				if (inchar > 0)
				{
					if (inputbufferptr == VGA_WIDTH)
						flushInput(&inputbuffer[0], &inputbufferptr);

					inputbuffer[inputbufferptr++] = inchar;
					
					if (inputbufferptr < VGA_WIDTH)
						inputbuffer[inputbufferptr] = '\0';
				}
				
				if (keycode == 28) // enter
					//terminal_writestring(&inputbuffer[0]);
					flushInput(&inputbuffer[0], &inputbufferptr);
				else if (keycode == 14) // backspace
					inputbuffer[--inputbufferptr] = '\0';
				
			}
		}
		
		updateInputRow(&inputbuffer[0]);
		update_cursor(VGA_HEIGHT - 1, inputbufferptr);
	}
}
