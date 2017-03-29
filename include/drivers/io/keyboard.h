#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define KEYS_COUNT 128
extern bool keyPressed[KEYS_COUNT];

// Special (non-character) keyboard keys
enum KEYS_SPECIAL
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

void keybd_init(void);
uint8_t keybd_readkey(void);
char scancodeToChar(const uint8_t scancode, const bool shiftPressed);
