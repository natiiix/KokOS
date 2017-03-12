#pragma once

#include "stdtypes.hpp"
#include "asm.hpp"

namespace keybd
{
	extern bool keypressed[128];

	void init(void);

	// Reads a scan code from keyboard and returns it
	uint8_t readkey(void);

	// Special (non-character) keyboard keys
	enum keys_special : uint8_t
	{
		KEY_ESCAPE = 1,
		KEY_BACKSPACE = 14,
		KEY_TAB = 15,
		KEY_ENTER = 28,
		KEY_CTRL = 29,
		KEY_SHIFT_LEFT = 42,
		KEY_SHIFT_RIGHT = 54,
		KEY_ALT = 56,
		KEY_CAPS_LOCK = 58,
		KEY_SCROLL_LOCK = 70,
	};
}
