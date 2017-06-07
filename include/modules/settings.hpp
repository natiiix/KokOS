#pragma once

#include <stdint.h>

namespace Settings
{
    extern uint8_t Shell_Background;
    extern uint8_t Shell_Foreground;

    extern uint8_t TextEditor_Background;
    extern uint8_t TextEditor_Foreground;

    void load(void);
    void save(void);
}
