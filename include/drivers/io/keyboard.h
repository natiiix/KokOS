#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "keyboard_global.h"

void keybd_init(void);
//char scancodeToChar(const uint8_t scancode, const bool shiftPressed);
//uint8_t keybd_readkey(void);
struct keyevent keybd_readevent(void);
