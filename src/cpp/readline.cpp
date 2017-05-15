#include <c/stdlib.h>
#include <c/stdio.h>
#include <c/string.h>

#include <cpp/string.hpp>

string readline(void)
{
    // Make sure the reading line is empty
    if (getcol() > 0)
    {
        newline();
    }

    size_t row = getrow();
    size_t maxLen = VGA_WIDTH - 1; // leave a space for the cursor at the end of the line

    string strInput;
    strInput.clear();

    while (true)
    {
        struct keyevent ke = readKeyEvent();
            
        if (ke.state)
        {
            if (ke.keychar > 0)
            {
                // Append a character to the input string
                strInput.push_back(ke.keychar);
            }
            else if (ke.scancode == KEY_ENTER && !ke.modifiers)
            {
                // Break the line and return the input
                newline();
                return strInput;
            }
            else if (ke.scancode == KEY_BACKSPACE && !ke.modifiers)
            {
                // Delete the last character in the input string
                if (strInput.size())
                {
                    strInput.pop_back();
                }
            }
            else if (ke.scancode == KEY_ESCAPE && !ke.modifiers)
            {
                // Clear the input string
                strInput.clear();
            }
        }

        // How many characters will be rendered
        size_t inRenderLen = (strInput.size() <= maxLen ? strInput.size() : maxLen);
        // Index of the first displayed character in the string
        size_t inStartIdx = strInput.size() - inRenderLen;

        // Don't attempt to print input if there is none
        if (inRenderLen)
        {
            string strInputRender = strInput.substr(inStartIdx, inRenderLen);
            sprintat(strInputRender, 0, row);
            strInputRender.dispose();
        }

        // How much empty space is there past the input string
        size_t emptySpace = VGA_WIDTH - inRenderLen;

        if (emptySpace > 0)
        {
            // Clear the rest of the screen row
            char* strspaces = strfill(' ', emptySpace);
            printat(strspaces, inRenderLen, row);
            delete strspaces;
        }

        setcursor(inRenderLen, row);
    }
}