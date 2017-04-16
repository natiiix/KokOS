#pragma once

#define KEYS_COUNT 128
extern bool keyPressed[KEYS_COUNT];

// Special (non-character) keyboard keys
enum KEYS_SPECIAL
{
	KEY_ESCAPE = 1,
	KEY_F1 = 59,
	KEY_F2 = 60,
	KEY_F3 = 61,
	KEY_F4 = 62,
	KEY_F5 = 63,
	KEY_F6 = 64,
	KEY_F7 = 65,
	KEY_F8 = 66,
	KEY_F9 = 67,
	KEY_F10 = 68,
	KEY_F11 = 87,
	KEY_F12 = 88,

	KEY_TAB = 15,
	KEY_BACKSPACE = 14,
	KEY_ENTER = 28,

	KEY_SHIFT_LEFT = 42,
	KEY_SHIFT_RIGHT = 54,
	KEY_CTRL = 29,
	KEY_ALT = 56,

	KEY_CAPS_LOCK = 58,
	KEY_NUM_LOCK = 69,
	KEY_SCROLL_LOCK = 70,

	KEY_INSERT = 82,
	KEY_DELETE = 83,
	KEY_HOME = 71,
	KEY_END = 79,
	KEY_PAGE_UP = 73,
	KEY_PAGE_DOWN = 81,

	KEY_ARROW_UP = 72,
	KEY_ARROW_RIGHT = 77,
	KEY_ARROW_DOWN = 80,
	KEY_ARROW_LEFT = 75,
};

#define KEY_DOWN true
#define KEY_UP false

#define MODIFIER_SHIFT 	0x01
#define MODIFIER_CTRL 	0x02
#define MODIFIER_ALT 	0x04

struct keyevent
{
	uint8_t scancode;
	bool state; // true = key has been pressed, false = key has been released
	uint8_t modifiers;
	char keychar;
};
