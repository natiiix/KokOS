#include "input.hpp"

namespace input
{
    char buffer[INPUT_BUFFER_SIZE];
    size_t bufferptr;

    void init(void)
    {        
        buffer[0] = '\0';
        bufferptr = 0;
    }

    // Returns char corresponding to specified key scan code
	char scancodeToChar(uint8_t scancode, bool shiftPressed)
	{
		// Scan code 57 is the last character key
		// Key with scan code above 57 is guaranteed to be non-character
		if (scancode > 57)
			return 0;

		return (shiftPressed ? asciiShift[scancode] : asciiDefault[scancode]);
	}

    void bufferpush(const char inchar)
    {
        if (bufferptr < INPUT_BUFFER_SIZE - 1)
        {
            buffer[bufferptr++] = inchar;    
            buffer[bufferptr] = '\0';
        }
    }

    bool handlekey(void)
    {
        uint8_t scancode = keybd::readkey();
        uint8_t keycode = scancode & 0b01111111;
        bool keystate = (scancode == keycode);

        if (keybd::keypressed[keycode] != keystate)
        {
            keybd::keypressed[keycode] = keystate;
            
            if (keystate)
            {
                char inchar = scancodeToChar(keycode, keybd::keypressed[keybd::KEY_SHIFT_LEFT] || keybd::keypressed[keybd::KEY_SHIFT_RIGHT]);
                
                if (inchar > 0)
                {
                    bufferpush(inchar);
                }
                else if (keycode == keybd::KEY_ENTER)
                    return false;
                else if (keycode == keybd::KEY_BACKSPACE)
                    // Make sure the cursor doesn't get past the beginning of the input row
                    if (bufferptr > 0)
                        buffer[--bufferptr] = '\0';
            }
        }
        
        return true;
    }
}
