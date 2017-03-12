#include "keyboard.hpp"

namespace keybd
{
	bool keypressed[128];
	
	void init(void)
	{
		for (size_t i = 0; i < 128; i++)
        {
            keypressed[i] = false;
        }
	}

	uint8_t readkey(void)
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
}
