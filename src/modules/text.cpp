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

size_t m_cursorCol;
size_t m_cursorRow;

size_t m_viewCol;
size_t m_viewRow;

vector<string> m_lines;
bool m_modified;

void generateLines(const uint8_t* const data)
{
    // Convert the data to a string object
    string strData = string((char*)data);

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

    // Clear the screen
    clear();

    for (size_t i = 0; i < VGA_HEIGHT && m_viewRow + i < m_lines.size(); i++)
    {
        // The line doesn't appear in the current view
        if (m_viewCol >= m_lines.at(m_viewRow + i).size())
        {
            continue;
        }

        // Print that part of the line that can be seen in the current view
        printat(&m_lines.at(m_viewRow + i).c_str()[m_viewCol], 0, i);
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
            if (ke.keychar > 0)
            {
                if (m_cursorCol < m_lines[m_cursorRow].size())
                {
                    m_lines[m_cursorRow].insert(ke.keychar, m_cursorCol);
                }
                else
                {
                    m_lines[m_cursorRow].push_back(ke.keychar);
                }

                m_cursorCol++;
            }
            /*else if (ke.scancode == KEY_ENTER && !ke.modifiers)
            {
                // Generate spaces to clear the input line on the screen
                char* strspaces = _generate_spaces(VGA_WIDTH);
                printat(strspaces, 0, row);
                delete strspaces;

                // Append the command string to the command history vector
                historyAppend(strInput);
                return strInput;
            }*/
            else if (ke.scancode == KEY_BACKSPACE && !ke.modifiers)
            {
                if (m_cursorCol && m_lines[m_cursorRow].size())
                {
                    if (m_cursorCol < m_lines[m_cursorRow].size())
                    {
                        m_lines[m_cursorRow].remove(m_cursorCol - 1);
                    }
                    else
                    {
                        m_lines[m_cursorRow].pop_back();
                    }

                    m_cursorCol--;
                }
                // TODO: append this line to the end of the previous line and pop it
            }
            else if (ke.scancode == KEY_DELETE && !ke.modifiers)
            {
                if (m_cursorCol < m_lines[m_cursorRow].size() - 1)
                {
                    m_lines[m_cursorRow].remove(m_cursorCol);
                }
                // TODO: append the next line to the end of this line and pop it
            }
            // Escape is used to exit the text editor 
            else if (ke.scancode == KEY_ESCAPE && !ke.modifiers)
            {
                break;
            }
            else if (ke.scancode == KEY_ARROW_LEFT && !ke.modifiers)
            {
                moveLeft();
            }
            else if (ke.scancode == KEY_ARROW_RIGHT && !ke.modifiers)
            {
                moveRight();
            }
            else if (ke.scancode == KEY_ARROW_UP && !ke.modifiers)
            {
                moveUp();
            }
            else if (ke.scancode == KEY_ARROW_DOWN && !ke.modifiers)
            {
                moveDown();
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

        generateLines(content);
        delete content;
    }
    // File doesn't exist
    else
    {
        // Set up the lines vector as if it were an empty file
        m_lines = vector<string>();
        m_lines.push_back(string());
    }

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
