#include <stdint.h>

namespace Settings
{
    uint8_t Shell_Background;
    uint8_t Shell_Foreground;

    uint8_t TextEditor_Background;
    uint8_t TextEditor_Foreground;
    
    void loadDefault(void)
    {
        // Black background
        Shell_Background = 0x0;
        // Grey foreground
        Shell_Foreground = 0x7;

        // Same as terminal
        TextEditor_Background = 0x0;
        TextEditor_Foreground = 0x7;
    }

    void load(void)
    {
        loadDefault();
    }
    
    void save(void)
    {

    }
}
