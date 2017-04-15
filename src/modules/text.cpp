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
        m_viewCol = (m_cursorCol - VGA_WIDTH);
    }

    // Top
    if (m_cursorRow < m_viewRow)
    {
        m_viewRow = m_cursorRow;
    }

    // Bottom
    if (m_cursorRow >= m_viewRow + VGA_HEIGHT)
    {
        m_viewRow = (m_cursorRow - VGA_HEIGHT);
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

    updateView();
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

    updateView();
}

void moveUp(void)
{
    // The cursor is already at the first line
    if (!m_cursorRow)
    {
        return;
    }

    m_cursorRow--;

    updateView();
}

void moveDown(void)
{
    // The cursor is already at the last line
    if (m_cursorRow == m_lines.size() - 1)
    {
        return;
    }

    m_cursorRow++;

    updateView();
}

void renderView(void)
{
    // Clear the screen
    clear();

    for (size_t i = 0; i < VGA_HEIGHT && m_viewRow + i < m_lines.size(); i++)
    {
        // The line doesn't appear in the current view
        if (m_viewCol >= m_lines.at(i).size())
        {
            continue;
        }

        printat(&m_lines.at(i).c_str()[m_viewCol], 0, i);
    }

    // Update the cursor location
    setcursor(m_cursorCol - m_viewCol, m_cursorRow - m_viewRow);
}

void editor(void)
{
    struct keyevent ke;

    while (true)
    {
        ke = readKeyEvent();

        if (ke.state)
        {
            /*if (ke.keychar > 0)
            {
                // Append a character to the input string
                strInput += ke.keychar;
            }
            else if (ke.scancode == KEY_ENTER && !ke.modifiers)
            {

                // Generate spaces to clear the input line on the screen
                char* strspaces = _generate_spaces(VGA_WIDTH);
                printat(strspaces, 0, row);
                delete strspaces;

                // Append the command string to the command history vector
                historyAppend(strInput);
                return strInput;
            }
            else if (ke.scancode == KEY_BACKSPACE && !ke.modifiers)
            {

                // Delete the last character in the input string
                if (strInput.size() > 0)
                {
                    strInput.pop_back();
                }
            }
            else if (ke.scancode == KEY_ESCAPE && !ke.modifiers)
            {

                // Clear the input string
                strInput.clear();
            }*/
 
            if (ke.scancode == KEY_ESCAPE && !ke.modifiers)
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
        }

        renderView();
    }
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
