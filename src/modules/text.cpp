#include <stddef.h>
#include <stdint.h>

#include <c/stdio.h>
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

// Color constants are pre-shifted so that they can be used without repetitive shifting
static const uint16_t COLOR_DEFAULT = 0x0700;

// Current cursor location relative to the first character in the file
size_t m_cursorCol;
size_t m_cursorRow;

// Current location of the first character on the screen relative to the first character in the file
size_t m_viewCol;
size_t m_viewRow;

vector<string> m_lines;
bool m_modified;

void generateLinesEmpty(void)
{
    m_lines = vector<string>();
    m_lines.push_back(string());
}

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

void generateData(uint8_t** const data, size_t* dataSize)
{
    // Join the lines to create a string containing all the data
    string strData = string::join(m_lines, '\n', false);

    // Set up the data variables
    *data = (uint8_t*)strData.c_str();
    *dataSize = strData.size();

    // strData must NOT be disposed now, because the cstring held by it is used when writing the file
}

void updateView(void)
{
    // The cursor is outside the screen boundaries
    // Move the view to fit the cursor on the screen

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

void moveLeft(void)
{
    // The cursor is already at the very left of the screen
    if (!m_cursorCol)
    {
        return;
        // TODO: Move it to the end of the previous line if available
    }

    m_cursorCol--;
}

void moveRight(void)
{
    // The cursor is at the very end of the line
    if (m_cursorCol == m_lines.at(m_cursorRow).size())
    {
        return;
        // TODO: Move it to the beginning of the next line if available
    }

    m_cursorCol++;
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
                vgaBuffer[vgaIdx++] = COLOR_DEFAULT | ' ';
            }
            else
            {
                // Write valid characters
                vgaBuffer[vgaIdx++] = COLOR_DEFAULT | strLine[i];
            }
        }
    }

    // Update the cursor location
    setcursor(m_cursorCol - m_viewCol, m_cursorRow - m_viewRow);
}

void editor(void)
{
    struct keyevent ke;

    m_cursorCol = 0;
    m_cursorRow = 0;

    m_viewCol = 0;
    m_viewRow = 0;

    m_modified = false;

    // Display the initial content of the file
    renderView();

    while (true)
    {
        ke = readKeyEvent();

        if (ke.state)
        {
            // Character Key
            if (ke.keychar > 0)
            {
                // If the cursor isn't at the end of the line
                if (m_cursorCol < m_lines[m_cursorRow].size())
                {
                    // Insert the character into the string
                    m_lines[m_cursorRow].insert(ke.keychar, m_cursorCol);
                }
                // If the cursor is at the very end of the line
                else
                {
                    // Append the character to the end of the line
                    m_lines[m_cursorRow].push_back(ke.keychar);
                }

                // Character has been added to the string, the cursor must be moved to the right as well
                m_cursorCol++;

                m_modified = true;
            }
            // Enter
            else if (ke.scancode == KEY_ENTER && !ke.modifiers)
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
            // Backspace
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
            // Escape
            else if (ke.scancode == KEY_ESCAPE && !ke.modifiers)
            {
                // Exit the editor by breaking the loop
                break;
            }
            // Left Arrow
            else if (ke.scancode == KEY_ARROW_LEFT && !ke.modifiers)
            {
                moveLeft();
            }
            // Right Arrow
            else if (ke.scancode == KEY_ARROW_RIGHT && !ke.modifiers)
            {
                moveRight();
            }
            // Up Arrow
            else if (ke.scancode == KEY_ARROW_UP && !ke.modifiers)
            {
                moveUp();
            }
            // Down Arrow
            else if (ke.scancode == KEY_ARROW_DOWN && !ke.modifiers)
            {
                moveDown();
            }
            // Home
            else if (ke.scancode == KEY_HOME && !ke.modifiers)
            {
                // Move the cursor to the beginning of the line
                m_cursorCol = 0;
            }
            // End
            else if (ke.scancode == KEY_END && !ke.modifiers)
            {
                // Move the cursor to the end of the line
                m_cursorCol = m_lines[m_cursorRow].size();
            }
            // Page Up
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

            renderView();
        }
    }

    // Clear the screen before leaving the editor
    // Otherwise the terminal would still display the content of the file
    // even though the editor has already been closed
    clear();
}

void cmd_text(const string& strArgs)
{
    vector<string> vecArgs = strArgs.split(' ', true);
    
    if (vecArgs.size() != 1)
    {
        print("Invalid arguments!\n");
        print("Syntax: text <File Path>\n");
        
        vecArgs.dispose();
        return;
    }

    struct FILE* file;

    // Find the file
    file = getFile(Shell::activePart, Shell::activeDir, vecArgs.at(0).c_str());

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
        // Set up the lines vector as if it were an empty file
        generateLinesEmpty();
    }

    // Lets us see all the debug messages that are displayed before the editors clears the screen
    debug_pause();

    // Start the editor itself
    editor();

    if (!m_modified)
    {
        debug_print("text.cpp | cmd_text() | The file hasn't been modified!");
        m_lines.dispose();
        vecArgs.dispose();
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
        vecArgs.dispose();
        return;
    }

    // Write data to file
    file = writeFile(Shell::activePart, Shell::activeDir, vecArgs[0].c_str(), data, dataSize);

    delete data;

    if (!file)
    {
        debug_print("text.cpp | cmd_text() | Failed to save the file!");
        vecArgs.dispose();
        return;
    }

    delete file;

    vecArgs.dispose();
}
