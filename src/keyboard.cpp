#include "keyboard.hpp"

namespace keybd
{
	uint8_t getScancode(void)
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
		while (true);
	}

	char scancodeToChar(uint8_t sc, bool shiftPressed)
	{
		// Scan code 57 is the last character key
		// Key with scan code above 57 is guaranteed to be non-character
		if (sc > 57)
			return 0;

		return (shiftPressed ? asciiShift[sc] : asciiDefault[sc]);
	}
}
