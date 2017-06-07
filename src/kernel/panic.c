#include <assembly.h>
#include <drivers/io/terminal.h>
#include <c/string.h>

void strcenter(const char* const strSource, char* const strOutput)
{
    size_t len = strlen(strSource);

    size_t linesOffset[VGA_HEIGHT];
    size_t linesLen[VGA_HEIGHT];
    size_t linesCount = 0;

    size_t currLineLen = 0;
    for (size_t i = 0; i < len && linesCount < VGA_HEIGHT - 1; i++)
    {
        if (currLineLen == 0)
        {
            linesOffset[linesCount] = i;
        }

        if (strSource[i] == '\n')
        {
            linesLen[linesCount] = currLineLen;
            linesCount++;            
            currLineLen = 0;
        }
        else
        {
            currLineLen++;

            if (currLineLen == VGA_WIDTH)
            {
                linesLen[linesCount] = currLineLen;
                linesCount++;
                currLineLen = 0;
            }
        }
    }

    if (currLineLen > 0)
    {
        linesLen[linesCount] = currLineLen;
        linesCount++;
    }

    size_t buffIdx = 0;

    size_t marginTop = (VGA_HEIGHT / 2) - (linesCount / 2);
    for (size_t i = 0; i < marginTop; i++)
    {
        strOutput[buffIdx++] = '\n';
    }

    for (size_t i = 0; i < linesCount; i++)
    {
        size_t marginLeft = (VGA_WIDTH / 2) - (linesLen[i] / 2) - (linesLen[i] % 2);
        for (size_t j = 0; j < marginLeft; j++)
        {
            strOutput[buffIdx++] = ' ';
        }

        for (size_t j = 0; j < linesLen[i]; j++)
        {
            strOutput[buffIdx++] = strSource[linesOffset[i] + j];
        }

        if (i < linesCount - 1 && linesLen[i] < VGA_WIDTH)
        {
            strOutput[buffIdx++] = '\n';
        }
    }

    strOutput[buffIdx] = '\0';
}

void kernel_panic(const char* const str)
{
    static const char strHeader[] = "Kernel Panic\n\n\n\n";
    static char strText[1024];
    static char strFormatted[2048];

    size_t textIdx = 0;

    // Copy the "Kernel Panic" header to the text string
    for (size_t i = 0; strHeader[i]; i++)
    {
        strText[textIdx++] = strHeader[i];
    }

    // Copy the panic message to the text string
    for (size_t i = 0; str[i]; i++)
    {
        strText[textIdx++] = str[i];
    }

    // Terminate the text string
    strText[textIdx] = '\0';

    // Choice of colors inspired by old Windows BSOD
    term_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE));
    term_clear();

    // Format the text by making it centered
    strcenter(strText, strFormatted);
    
    // Write the panic message to the screen
    term_write(strFormatted, false);

    // Move the cursor outside of the screen
    term_setcursor(0, 25);

    // Keep the processor in halted state
    while (true)
    {
        hlt();
    }
}
