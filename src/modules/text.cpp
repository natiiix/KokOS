#include <stddef.h>
#include <stdint.h>

#include <c/stdio.h>
#include <c/string.h>
#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#include <drivers/io/terminal_global.h>
#include <drivers/io/keyboard_global.h>

#include <drivers/storage/fat.h>
#include <modules/shell_global.hpp>

#include <kernel.h>

// We want to access the VGA buffer directly to make the screen updating faster
extern uint16_t* vgaBuffer;

// Specifies by how many lines do Page Up / Page Down keys shift the view
static const size_t PAGE_UP_DOWN_LINES = 10;

static const size_t MENU_PADDING_LEFT = 24;

static const uint8_t MENU_COLOR_DEFAULT_BG = 0x0;
static const uint8_t MENU_COLOR_DEFAULT_FG = 0x7;
static const uint8_t MENU_COLOR_SELECTED_BG = 0x7;
static const uint8_t MENU_COLOR_SELECTED_FG = 0x0;

// Current cursor location relative to the first character in the file
size_t m_cursorCol;
size_t m_cursorRow;

// Current location of the first character on the screen relative to the first character in the file
size_t m_viewCol;
size_t m_viewRow;

vector<string> m_lines;
bool m_modified;

bool m_renderRequired;

// Used for VGA buffer filling
uint16_t m_colorScheme;
uint8_t m_colBG;
uint8_t m_colFG;

enum EDITOR_SCREEN
{
    SCREEN_EXIT, // indicates the intent to exit the editor
    SCREEN_TEXT, // text editing mode
    SCREEN_MENU, // user is in menu
};

// Specified which screen is currently being rendered
EDITOR_SCREEN m_screen;

size_t m_menuLine; // index of currently selected line in the menu
static const size_t MENU_LINE_COUNT = 4; // the total number of lines in menu

// Used when the text file is empty / doesn't exist
// Generates a single empty line in the line vector
void generateLinesEmpty(void)
{
    m_lines = vector<string>();
    m_lines.push_back(string());
}

// Used when the text file exists and isn't empty
// Generates line vector from the data read from the file
void generateLines(const uint8_t* const data)
{
    // Convert the data to a string object
    string strData;
    strData.push_back((char*)data);

    // Split the string into separate lines
    m_lines = strData.split('\n', false);

    // Dispose the data string, because it was only meant to be used termporarily
    strData.dispose();
}

// Used when saving modified content of the file
// Converts the line vector into linear data bytes
void generateData(uint8_t** const data, size_t* dataSize)
{
    // Join the lines to create a string containing all the data
    string strData = string::join(m_lines, '\n', false);

    // Set up the data variables
    *data = (uint8_t*)strData.c_str();
    *dataSize = strData.size();

    // strData must NOT be disposed now, because the cstring held by it is used when writing the file
}

// Used in editor mode
// Updates the editor view so that the cursor is on the screen
// If the cursor is on the streen it does nothing
void updateView(void)
{
    // Left
    if (m_cursorCol < m_viewCol)
    {
        m_viewCol = m_cursorCol;
    }

    // Right
    if (m_cursorCol >= m_viewCol + VGA_WIDTH)
    {
        m_viewCol = m_cursorCol - VGA_WIDTH + 1;
    }

    // Top
    if (m_cursorRow < m_viewRow)
    {
        m_viewRow = m_cursorRow;
    }

    // Bottom
    if (m_cursorRow >= m_viewRow + VGA_HEIGHT)
    {
        m_viewRow = m_cursorRow - VGA_HEIGHT + 1;
    }
}

// Used when left arrow key is pressed
// Moves cursor to the left
void moveLeft(void)
{
    // The cursor is not at the beginning of the line
    if (m_cursorCol)
    {
        // Move the cursor to the left
        m_cursorCol--;
    }
    // The cursor is already at the very left of the screen
    else
    {
        // Move the cursor to the end of the previous line unless this is the first line
        if (m_cursorRow)
        {
            m_cursorRow--;
            m_cursorCol = m_lines.at(m_cursorRow).size();
        }
    }
}

// Used when right arrow key is pressed
// Moves cursor to the right
void moveRight(void)
{
    // The cursor is not end the end of the line
    if (m_cursorCol < m_lines.at(m_cursorRow).size())
    {
        m_cursorCol++;
    }
    // The cursor is at the very end of the line
    else
    {
        // If this is not the last line yet
        // Move to the beginning of the next line
        if (m_cursorRow < m_lines.size() - 1)
        {
            m_cursorRow++;
            m_cursorCol = 0;
        }
    }
}

// The "Flying Cursor" is what happens when the cursor is moved up/down to a line
// that is shorter than the cursor's previous horizontal position,
// thus rendering it further to the right from the actual end of the line
void fixFlyingCursor(void)
{
    if (m_cursorCol > m_lines.at(m_cursorRow).size())
    {
        m_cursorCol = m_lines.at(m_cursorRow).size();
    }
}

// Used when up arrow key is pressed
// Moves cursor one line up if possible
void moveUp(void)
{
    // The cursor is already at the first line
    if (!m_cursorRow)
    {
        return;
    }

    m_cursorRow--;

    fixFlyingCursor();
}

// Used when down arrow key is pressed
// Moves cursor one line down if possible
void moveDown(void)
{
    // The cursor is already at the last line
    if (m_cursorRow == m_lines.size() - 1)
    {
        return;
    }

    m_cursorRow++;

    fixFlyingCursor();
}

// Used in editor mode
// Displays the current content of line within the view on screen
void renderView(void)
{
    // Update the view-related information
    updateView();

    // Index within the VGA buffer
    size_t vgaIdx = 0;

    for (size_t i = 0; i < VGA_HEIGHT; i++)
    {
        bool endOfLine = false;

        // There is not enough lines
        if (m_viewRow + i >= m_lines.size() ||
        // The line doesn't appear in the current view
            m_viewCol >= m_lines.at(m_viewRow + i).size())
        {
            endOfLine = true;
        }

        // Pointer to the line string starting at the current view
        char* strLine = &m_lines.at(m_viewRow + i).c_str()[m_viewCol];

        for (size_t i = 0; i < VGA_WIDTH; i++)
        {
            // End of line reached
            if (strLine[i] == '\0')
            {
                endOfLine = true;
            }

            if (endOfLine)
            {
                // Fill the rest of the line with spaces
                vgaBuffer[vgaIdx++] = m_colorScheme | ' ';
            }
            else
            {
                // Write valid characters
                vgaBuffer[vgaIdx++] = m_colorScheme | strLine[i];
            }
        }
    }

    // Update the cursor location
    setcursor(m_cursorCol - m_viewCol, m_cursorRow - m_viewRow);

    m_renderRequired = false;
}

// Used in menu mode
// Changes the active color to one with a higher value
uint8_t colorUp(const uint8_t oldColor)
{
    if (oldColor == 0xF)
    {
        // Color overflow
        return 0x0;
    }
    else
    {
        // Next color in the chain
        return oldColor + 1;
    }
}

// Used in menu mode
// Changes the active color to one with a lower value
uint8_t colorDown(const uint8_t oldColor)
{
    if (oldColor == 0x0)
    {
        // Color underflow
        return 0xF;
    }
    else
    {
        // Previous color in the chain
        return oldColor - 1;
    }
}

// Used in menu mode
// Generates a cstring containing the name of a color from its value
char* colorToStr(const uint8_t color)
{
    char* strColor = (char*)malloc(32);

    switch (color)
    {
        case 0x0:
            strcopy("Black", strColor);
            break;

        case 0x1:
            strcopy("Blue", strColor);
            break;
            
        case 0x2:
            strcopy("Green", strColor);
            break;
            
        case 0x3:
            strcopy("Cyan", strColor);
            break;
            
        case 0x4:
            strcopy("Red", strColor);
            break;
            
        case 0x5:
            strcopy("Magenta", strColor);
            break;
            
        case 0x6:
            strcopy("Brown", strColor);
            break;
            
        case 0x7:
            strcopy("Light Grey", strColor);
            break;
            
        case 0x8:
            strcopy("Dark Grey", strColor);
            break;
            
        case 0x9:
            strcopy("Light Blue", strColor);
            break;
            
        case 0xA:
            strcopy("Light Green", strColor);
            break;
            
        case 0xB:
            strcopy("Light Cyan", strColor);
            break;
            
        case 0xC:
            strcopy("Light Red", strColor);
            break;
            
        case 0xD:
            strcopy("Light Magenta", strColor);
            break;
            
        case 0xE:
            strcopy("Light Brown", strColor);
            break;
            
        case 0xF:
            strcopy("White", strColor);
            break;

        default:
            strcopy("Unknown", strColor);
            break;
    }

    return strColor;
}

// Used when exitting menu mode
// Updates the color scheme used in editor mode
void updateColorScheme(void)
{
    m_colorScheme = (((uint16_t)m_colBG) << 12) | (((uint16_t)m_colFG) << 8);
}

// Used in menu mode
// Sets render colors according to whether the rendered line is currently selected or not
void setMenuLineColor(const size_t renderLine)
{
    if (renderLine == m_menuLine)
    {
        setcolor((VGA_COLOR)MENU_COLOR_SELECTED_FG, (VGA_COLOR)MENU_COLOR_SELECTED_BG);
    }
    else
    {
        setcolor((VGA_COLOR)MENU_COLOR_DEFAULT_FG, (VGA_COLOR)MENU_COLOR_DEFAULT_BG);
    }
}

// Used in menu mode
// Renders the content of the menu screen
void renderMenu()
{
    clear(); // clear the screen
    setcursor(0, 25); // put the cursor out of the screen

    // Header line
    size_t lineIdx = 2;
    printat("  -- MENU --", MENU_PADDING_LEFT, lineIdx);

    // Background Color setting line
    lineIdx += 4;
    setMenuLineColor(0);
    printat("Background Color: ", MENU_PADDING_LEFT, lineIdx);
    char* strBG = colorToStr(m_colBG);
    printat(strBG, MENU_PADDING_LEFT + 18, lineIdx);
    delete strBG;

    // Foreground Color setting line
    lineIdx += 2;
    setMenuLineColor(1);
    printat("Foreground Color: ", MENU_PADDING_LEFT, lineIdx);
    char* strFG = colorToStr(m_colFG);
    printat(strFG, MENU_PADDING_LEFT + 18, lineIdx);
    delete strFG;

    // Save and Exit button
    lineIdx += 4;
    setMenuLineColor(2);
    printat("Save and Exit", MENU_PADDING_LEFT, lineIdx);

    // Exit Without Saving button
    lineIdx += 2;
    setMenuLineColor(3);
    printat("Exit Without Saving", MENU_PADDING_LEFT, lineIdx);

    // Current line and column numbers are written at the bottom of the screen
    lineIdx = 23;
    setcolor((VGA_COLOR)MENU_COLOR_DEFAULT_FG, (VGA_COLOR)MENU_COLOR_DEFAULT_BG);
    char* strRow = tostr(m_cursorRow, 10);
    printat("Line: ", MENU_PADDING_LEFT, lineIdx);
    printat(strRow, MENU_PADDING_LEFT + 6, lineIdx);
    delete strRow;

    setcolor((VGA_COLOR)MENU_COLOR_DEFAULT_FG, (VGA_COLOR)MENU_COLOR_DEFAULT_BG);
    char* strCol = tostr(m_cursorCol, 10);
    printat("Column: ", MENU_PADDING_LEFT + 16, lineIdx);
    printat(strCol, MENU_PADDING_LEFT + 24, lineIdx);
    delete strCol;

    m_renderRequired = false;
}

// Used in editor mode
// Inserts a character at the current cursor position
void editorInsertChar(const char c)
{
    // If the cursor isn't at the end of the line
    if (m_cursorCol < m_lines[m_cursorRow].size())
    {
        // Insert the character into the string
        m_lines[m_cursorRow].insert(c, m_cursorCol);
    }
    // If the cursor is at the very end of the line
    else
    {
        // Append the character to the end of the line
        m_lines[m_cursorRow].push_back(c);
    }

    // Character has been added to the string, the cursor must be moved to the right as well
    m_cursorCol++;

    m_modified = true;
}

// Used in editor mode
// Creates a new line at the current cursor position
// If there was any text on the right of the cursor, it will be moved to the new line
void editorNewLine(void)
{
    // The cursor is at the very beginning of the line
    // We can make things easier by simple inserting an empty life before the current line
    // instead of adding one after it and then copying the entire line and clearing it
    if (!m_cursorCol)
    {
        // Insert a new empty line before the current line
        m_lines.insert(string(), m_cursorRow);

        // Move the cursor to the new line
        m_cursorRow++;
    }
    else
    {
        // This is the last line
        if (m_cursorRow + 1 == m_lines.size())
        {
            // Append an empty line at the end of the vector
            m_lines.push_back(string());
        }
        // This isn't the last line
        else
        {
            // Insert a new empty line after this line
            m_lines.insert(string(), m_cursorRow + 1);
        }

        // The cursor is somewhere in the middle of the line
        // The part of the line on the right the cursor must be copied to a new line and removed from the old line
        if (m_cursorCol < m_lines[m_cursorRow].size())
        {
            // Copy the part of this line after the cursor to the new empty line
            m_lines[m_cursorRow + 1].push_back(&m_lines[m_cursorRow].c_str()[m_cursorCol]);
            // Remove that part of the line from this line
            m_lines[m_cursorRow].remove(m_cursorCol, m_lines[m_cursorRow].size() - m_cursorCol);
        }

        // Move the cursor to the new line
        m_cursorRow++;
        // When a new line is created the cursor must be at its beginning
        m_cursorCol = 0;
    }

    m_modified = true;
}

// Used in editor mode
// Counts the amount of leading spaces on a specified line
size_t editorCountSpaces(const size_t line)
{
    size_t spaceCount = 0;
    size_t prevLineLen = m_lines.at(line).size();
    
    for (size_t i = 0; i < prevLineLen; i++)
    {
        // Character is a space
        if (m_lines.at(line).at(i) == ' ')
        {
            spaceCount++;
        }
        // Character is NOT a space
        else
        {
            // Stop the counting
            break;
        }
    }

    return spaceCount;
}

// Used in editor mode
// Handles keypresses and re-renders the screen whenever necessary
void screenText(void)
{
    struct keyevent ke;

    while (true)
    {
        if (m_renderRequired)
        {
            renderView();
        }

        ke = readKeyEvent();

        if (ke.state)
        {
            // Character Key
            if (ke.keychar > 0)
            {
                editorInsertChar(ke.keychar);
            }
            // Enter
            else if (ke.scancode == KEY_ENTER && !ke.modifiers)
            {
                editorNewLine();
            }
            // Backspace
            // Deletes the character before the cursor is there is any
            else if (ke.scancode == KEY_BACKSPACE && !ke.modifiers)
            {
                // If the cursor isn't at the beginning of the line
                if (m_cursorCol)
                {
                    // The cursor isn't at the end of the line
                    if (m_cursorCol < m_lines[m_cursorRow].size())
                    {
                        // Delete the character right before the cursor
                        m_lines[m_cursorRow].remove(m_cursorCol - 1);
                    }
                    // The cursor is at the very end of the line
                    else
                    {
                        // Pop the last character from the string
                        m_lines[m_cursorRow].pop_back();
                    }

                    // The character has been deleted, therefore the cursor must be moved to the left as well
                    m_cursorCol--;
                }
                // The cursor is at the very beginning of the line
                // It can't be the very first line, because we wouldn't have a line to append this line to
                else if (m_cursorRow)
                {
                    size_t prevLineOldLen = m_lines[m_cursorRow - 1].size();

                    // Append this line to the end of the previous line
                    m_lines[m_cursorRow - 1].push_back(m_lines[m_cursorRow]);
                    // Pop this line from the vector
                    m_lines.remove(m_cursorRow);

                    // Move the cursor to the previous line
                    m_cursorRow--;
                    // Move the cursor to the former end of the previous line, right before the part that was just appended
                    m_cursorCol = prevLineOldLen;
                }

                m_modified = true;
            }
            // Delete
            // Deletes the character after the cursor if there is any
            else if (ke.scancode == KEY_DELETE && !ke.modifiers)
            {
                // The cursor isn't at the end of the line
                if (m_cursorCol + 1 < m_lines[m_cursorRow].size())
                {
                    // Remove the character at the current cursor location
                    m_lines[m_cursorRow].remove(m_cursorCol);
                }
                // The cursor is right before the last character
                else if (m_cursorCol + 1 == m_lines[m_cursorRow].size())
                {
                    // Pop the last character in the string
                    m_lines[m_cursorRow].pop_back();
                }
                // The cursor is at the very end of the line
                // It can't be the very last line, because we wouldn't have anything to append to it
                else if (m_cursorRow < m_lines.size() - 1)
                {
                    // Append the next line to the end of this line
                    m_lines[m_cursorRow].push_back(m_lines[m_cursorRow + 1]);
                    // Pop the next line from the vector
                    m_lines.remove(m_cursorRow + 1);
                }

                m_modified = true;
            }
            // Ctrl + Delete
            // Deletes the whole current line
            else if (ke.scancode == KEY_DELETE && ke.modifiers == MODIFIER_CTRL)
            {
                // If the current line isn't the last line of the file
                if (m_lines.size() > m_cursorRow + 1)
                {
                    // Delete the current line
                    m_lines.remove(m_cursorRow);
                }
                // The current line is the last line of the file
                else
                {
                    // Clear the line
                    m_lines[m_cursorRow].clear();
                }

                // Move the cursor to the beginning of the line
                m_cursorCol = 0;

                m_modified = true;
            }
            // Escape
            // Switches from editor mode to menu mode
            else if (ke.scancode == KEY_ESCAPE && !ke.modifiers)
            {
                // Switch to the settings screen
                m_screen = SCREEN_MENU;
                break;
            }
            // Left Arrow
            // Moves the cursor one character back
            else if (ke.scancode == KEY_ARROW_LEFT && !ke.modifiers)
            {
                moveLeft();
            }
            // Right Arrow
            // Moves the cursor one character forward
            else if (ke.scancode == KEY_ARROW_RIGHT && !ke.modifiers)
            {
                moveRight();
            }
            // Up Arrow
            // Moves the cursor one line up
            else if (ke.scancode == KEY_ARROW_UP && !ke.modifiers)
            {
                moveUp();
            }
            // Down Arrow
            // Moves the cursor one line down
            else if (ke.scancode == KEY_ARROW_DOWN && !ke.modifiers)
            {
                moveDown();
            }
            // Home
            // Moves the cursor to the beginning of the line
            else if (ke.scancode == KEY_HOME && !ke.modifiers)
            {
                // Count leading spaces on this line
                size_t leadingSpaces = editorCountSpaces(m_cursorRow);

                // If the cursor is at the end of the leading spaces
                if (m_cursorCol == leadingSpaces)
                {
                    // Move it to the very beginning of the line
                    m_cursorCol = 0;
                }
                // If the cursor is anywhere else
                else
                {
                    // Move it to past the leading spaces
                    m_cursorCol = leadingSpaces;
                }
            }
            // End
            // Moves the cursor to the end of the current line
            else if (ke.scancode == KEY_END && !ke.modifiers)
            {
                // Move the cursor to the end of the line
                m_cursorCol = m_lines[m_cursorRow].size();
            }
            // Page Up
            // Scrolls the view by N lines up (doesn't move the cursor unless necessary)            
            else if (ke.scancode == KEY_PAGE_UP && !ke.modifiers)
            {
                // If there are lines above the current view
                if (m_viewRow)
                {
                    // If there's more than N lines above the view
                    if (m_viewRow > PAGE_UP_DOWN_LINES)
                    {
                        // Move the view N lines up
                        m_viewRow -= PAGE_UP_DOWN_LINES;
                    }
                    else
                    {
                        // Move the view to the first line
                        m_viewRow = 0;
                    }

                    // If the cursor is outside the screen
                    if (m_cursorRow >= m_viewRow + VGA_HEIGHT)
                    {
                        // Move the cursor up to last line of the view
                        m_cursorRow = m_viewRow + VGA_HEIGHT - 1;

                        fixFlyingCursor();
                    }
                }
            }
            // Page Down
            // Scrolls the view by N lines down (doesn't move the cursor unless necessary)
            else if (ke.scancode == KEY_PAGE_DOWN && !ke.modifiers)
            {
                // If the view can be moved lower
                if (m_viewRow + VGA_HEIGHT < m_lines.size())
                {
                    // If there is more than N lines below the current view
                    if (m_lines.size() - (m_viewRow + VGA_HEIGHT) > PAGE_UP_DOWN_LINES)
                    {
                        // Move the view N lines down
                        m_viewRow += PAGE_UP_DOWN_LINES;
                    }
                    else
                    {
                        // Move the view all the way down
                        m_viewRow = m_lines.size() - VGA_HEIGHT;
                    }

                    // If the cursor is outside the screen
                    if (m_cursorRow < m_viewRow)
                    {
                        // Move the cursor down to the first line of the view
                        m_cursorRow = m_viewRow;

                        fixFlyingCursor();
                    }
                }
            }
            // Tab
            // Used for convenient alignment of text (mainly when programming)
            // Text will be aligned by an even number of spaces (2, 4, 6, 8,...)
            else if (ke.scancode == KEY_TAB && !ke.modifiers)
            {
                bool tabAligned = false;

                // Cursor is NOT on the first line
                // Try to align this line with the previous line
                if (m_cursorRow)
                {
                    // Count the number of leading spaces on the previous line
                    size_t spaceCount = editorCountSpaces(m_cursorRow - 1);

                    // There are spaces beyond the current cursor position on the previous line
                    if (m_cursorCol < spaceCount)
                    {
                        // Insert the number of spaces required to make the lines aligned
                        size_t spacesToInsert = spaceCount - m_cursorCol;

                        for (size_t i = 0; i < spacesToInsert; i++)
                        {
                            editorInsertChar(' ');
                        }

                        // Line has been aligned with the previous line
                        tabAligned = true;
                    }
                }

                // Couldn't align this line with the previous
                // This is the first line or the cursor is further than the last space on the previous line
                if (!tabAligned)
                {
                    // At least one space is always inserted
                    editorInsertChar(' ');

                    // Insert another space if the cursor isn't 2 space aligned
                    if (m_cursorCol % 2)
                    {
                        editorInsertChar(' ');
                    }
                }
            }
            // Shift + Tab
            // Used to decrease the number of leading spaces
            // Removes just 1 space if there is an odd number of them, 2 spaces if the number is even
            else if (ke.scancode == KEY_TAB && ke.modifiers == MODIFIER_SHIFT)
            {
                // Count the number of leading spaces on this line
                size_t spaceCount = editorCountSpaces(m_cursorRow);

                // Can't decrease the number of leading spaces if there are none
                if (spaceCount)
                {                    
                    // Make the number of leading space even again
                    size_t spacesToRemove = 2 - (spaceCount % 2);
                    m_lines[m_cursorRow].remove(0, spacesToRemove);

                    // Also move the cursor accordingly
                    if (m_cursorCol > spacesToRemove)
                    {
                        m_cursorCol -= spacesToRemove;
                    }
                    else
                    {
                        m_cursorCol = 0;
                    }
                }
            }

            m_renderRequired = true;
        }
    }
}

// Used in menu mode
// Handles keypresses and re-renders the screen whenever necessary
void screenMenu(void)
{
    struct keyevent ke;

    while (true)
    {
        if (m_renderRequired)
        {
            renderMenu();
        }

        ke = readKeyEvent();

        if (ke.state)
        {
            // Escape
            if (ke.scancode == KEY_ESCAPE && !ke.modifiers)
            {
                // Update the color scheme according to the new color settings
                updateColorScheme();

                // Switch back to the text editing screen
                m_screen = SCREEN_TEXT;
                break;
            }
            // Left Arrow
            else if (ke.scancode == KEY_ARROW_LEFT && !ke.modifiers)
            {
                switch (m_menuLine)
                {
                    case 0:
                        m_colBG = colorDown(m_colBG);
                        break;

                    case 1:
                        m_colFG = colorDown(m_colFG);
                        break;

                    default:
                        break;
                }
            }
            // Right Arrow
            else if (ke.scancode == KEY_ARROW_RIGHT && !ke.modifiers)
            {
                switch (m_menuLine)
                {
                    case 0:
                        m_colBG = colorUp(m_colBG);
                        break;

                    case 1:
                        m_colFG = colorUp(m_colFG);
                        break;

                    default:
                        break;
                }
            }
            // Up Arrow
            else if (ke.scancode == KEY_ARROW_UP && !ke.modifiers)
            {
                // Up arrow pressed at the very top line
                if (m_menuLine == 0)
                {
                    m_menuLine = MENU_LINE_COUNT - 1;
                }
                else
                {
                    m_menuLine--;
                }
            }
            // Down Arrow
            else if (ke.scancode == KEY_ARROW_DOWN && !ke.modifiers)
            {
                m_menuLine++;

                // Down arrow pressed at the very bottom line
                if (m_menuLine >= MENU_LINE_COUNT)
                {
                    m_menuLine = 0;
                }
            }
            // Enter
            else if (ke.scancode == KEY_ENTER && !ke.modifiers)
            {
                if (m_menuLine == 2 || m_menuLine == 3)
                {
                    // User wishes not to save any modifications made to the text file
                    if (m_menuLine == 3)
                    {
                        m_modified = false;
                    }

                    // Exit the editor
                    m_screen = SCREEN_EXIT;
                    break;
                }
            }

            m_renderRequired = true;
        }
    }
}

// Used when entering editor mode for the first time
// Set all the variable to their default values
void presetVariables(void)
{
    m_cursorCol = 0;
    m_cursorRow = 0;

    m_viewCol = 0;
    m_viewRow = 0;

    m_modified = false;

    // Same as terminal (black background, grey foreground)
    m_colBG = 0x0;
    m_colFG = 0x7;
    updateColorScheme();

    m_screen = SCREEN_TEXT;

    m_menuLine = 0;
}

// Loop that keeps the editor alive
// The loop is broken once the user requests an exit via menu
void editor(void)
{
    presetVariables();

    while (true)
    {
        // The screen must always be rendered after switching between screens
        m_renderRequired = true;

        // Editor menu
        if (m_screen == SCREEN_TEXT)
        {
            screenText();
        }
        // Menu mode
        else if (m_screen == SCREEN_MENU)
        {
            screenMenu();
        }
        // Exit requested
        else
        {
            break;
        }
    }

    // Clear the screen before leaving the editor
    // Otherwise the terminal would still display the content of the file
    // even though the editor has already been closed
    clear();
}

void textEditorStart(const char* const filePath)
{
    // Find the file
    struct FILE* file = getFile(Shell::activePart, Shell::activeDir, filePath);

    // File exists
    if (file)
    {
        // Read file's content
        uint8_t* content = readFile(file);
        delete file;

        // If the file has some content (isn't empty)
        if (content)
        {
            generateLines(content);
            delete content;
        }
        // The file is empty
        else
        {
            generateLinesEmpty();
        }
    }
    // File doesn't exist
    else
    {
        // Make the file path doesn't lead to a non-existent directory path
        // All text written in such scenario would go to waste because such file couldn't be saved
        if (!dirPathValid(Shell::activePart, Shell::activeDir, filePath))
        {
            print("Invalid directory path!\n");
            return;
        }

        // Set up the lines vector as if it were an empty file
        generateLinesEmpty();
    }

    // Lets us see all the debug messages that are displayed before the editors clears the screen
    debug_memusage();
    debug_pause();

    // Start the editor itself
    editor();

    if (!m_modified)
    {
        debug_print("text.cpp | cmd_text() | The file hasn't been modified!");
        m_lines.dispose();
        return;
    }

    uint8_t* data = nullptr;
    size_t dataSize = 0;

    // Generate data byte array from the lines
    generateData(&data, &dataSize);
    // Dispose the lines vector
    m_lines.dispose();

    if (!data)
    {
        debug_print("text.cpp | cmd_text() | Failed to generate data from lines!");
        return;
    }

    // Write data to file
    file = writeFile(Shell::activePart, Shell::activeDir, filePath, data, dataSize);

    delete data;

    if (!file)
    {
        debug_print("text.cpp | cmd_text() | Failed to save the file!");
        return;
    }

    delete file;
}
