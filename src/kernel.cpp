/* Surely you will remove the processor conditionals and this comment
   appropriately depending on whether or not you use C++. */
#if !defined(__cplusplus)
#include <stdbool.h> /* C doesn't have booleans by default. */
#endif
#include <stddef.h>
#include <stdint.h>

#include "terminal.hpp"
#include "keyboard.hpp"

/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

size_t strlen(const char* str)
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
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
	terminal_writeline(&inbuff[0]); // flush the buffer	
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
				char inchar = scancodeToChar(keycode, keypressed[KEY_SHIFT_LEFT] || keypressed[KEY_SHIFT_RIGHT]);
				
				if (inchar > 0)
				{
					if (inputbufferptr == VGA_WIDTH)
						flushInput(&inputbuffer[0], &inputbufferptr);

					inputbuffer[inputbufferptr++] = inchar;
					
					if (inputbufferptr < VGA_WIDTH)
						inputbuffer[inputbufferptr] = '\0';
				}
				
				if (keycode == KEY_ENTER)
					flushInput(&inputbuffer[0], &inputbufferptr);
				else if (keycode == KEY_BACKSPACE)
					// Make sure the cursor doesn't get past the beginning of the input row
					if (inputbufferptr > 0)
						inputbuffer[--inputbufferptr] = '\0';
			}
		}
		
		updateInputRow(&inputbuffer[0]);
		terminal_setcursor(VGA_HEIGHT - 1, inputbufferptr);
	}
}
