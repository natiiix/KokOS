#pragma once

#include <stddef.h>
#include <stdint.h>

class string;

class TextEditor
{
public:
    void init(const string& strArgs);

private:
    void edit(void);

    size_t m_cursorRow;
    size_t m_cursorCol;

    size_t m_viewRow;
    size_t m_viewCol;
};
