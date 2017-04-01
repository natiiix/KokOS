#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "keyboard_global.h"

void keybd_init(void);
struct keyevent keybd_read(void);
