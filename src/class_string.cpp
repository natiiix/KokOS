#include "class_string.hpp"

string::string(void) :
    m_ptr(mem::dynalloc(1)),
    m_ptrC((char*)m_ptr),
    m_size(0)
{    
    fixend();
}

string::string(const char* const str)
{
    size_t strlength = cstr::len(str);
    
    string::resize(strlength);
    
    for (size_t i = 0; i < strlength; i++)
    {
        m_ptrC[i] = str[i];
    }
}

void string::dispose(void)
{
    delete m_ptrC;
}

// Capacity
size_t string::size(void)
{
    return m_size;
}
//
void string::resize(const size_t newsize)
{
    updatePtr(mem::dynresize(m_ptr, (m_size = newsize) + 1));
    fixend();
}
//
void string::clear(void)
{
    string::resize(0);
}
//
bool string::empty(void)
{
    return !m_size;
}

// Element access
char& string::operator[](const size_t idx)
{
    return string::at(idx);
}
//
char& string::at(const size_t idx)
{
    return m_ptrC[idx];
}
//
char& string::front(void)
{
    return m_ptrC[0];
}
//
char& string::back(void)
{
    return m_ptrC[m_size - 1];
}

// Modifiers
void string::append(const char c)
{
    string::push_back(c);
}
//
void string::append(const char* const str)
{
    string::push_back(str);
}
//
void string::append(const string& str)
{
    string::push_back(str);
}
//
void string::push_back(const char c)
{
    string::resize(m_size + 1);
    m_ptrC[m_size - 1] = c;
}
//
void string::push_back(const char* const str)
{
    size_t oldsize = m_size;
    size_t strlength = cstr::len(str);
    
    string::resize(m_size + strlength);
    
    for (size_t i = 0; i < strlength; i++)
    {
        m_ptrC[oldsize + i] = str[i];
    }
}
//
void string::push_back(const string& str)
{
    string::push_back(str.m_ptrC);
}
//
void string::pop_back(void)
{
    string::resize(m_size - 1);
}

// String operations
char* string::c_str(void)
{
    return (char*)m_ptr;
}
//
string string::substr(const size_t pos, const size_t len)
{
    string strout;

    if (pos >= m_size)
    {
        return strout;
    }

    // If len is 0 or higher than the number of remaining
    // characters the rest of the string is returned
    size_t copylen = len;
    size_t remainchar = m_size - pos;

    if (len == 0 || len > remainchar)
    {
        copylen = remainchar;
    }

    strout.resize(copylen);

    for (size_t i = 0; i < copylen; i++)
    {
        strout[i] = m_ptrC[pos + i];
    }

    return strout;
}
//
bool string::compare(const string& str)
{
    // If they have share a pointer they must be the same
    if (str.m_ptrC == m_ptrC)
        return true;

    // Different size implies they must be different
    if (str.m_size != m_size)
        return false;

    for (size_t i = 0; i < m_size; i++)
    {
        if (str.m_ptrC[i] != m_ptrC[i])
            return false;
    }

    return true;
}
//
string string::tolower(void)
{
    string strout(m_ptrC);

    for (size_t i = 0; i < strout.m_size; i++)
    {
        if (strout.m_ptrC[i] >= 'A' &&
            strout.m_ptrC[i] <= 'Z')
        {
            strout.m_ptrC[i] += 'a' - 'A';
        }
    }

    return strout;
}
//
string string::toupper(void)
{
    string strout(m_ptrC);

    for (size_t i = 0; i < strout.m_size; i++)
    {
        if (strout.m_ptrC[i] >= 'a' &&
            strout.m_ptrC[i] <= 'z')
        {
            strout.m_ptrC[i] += 'A' - 'a';
        }
    }

    return strout;
}

// Operator overloads
bool string::operator==(const string& str)
{
    return string::compare(str);
}
//
string& string::operator+=(const string& str)
{
    string::push_back(str);
    return *this;
}
//
string string::operator+(const string& str)
{
    string strout(m_ptrC);
    strout.push_back(str.m_ptrC);
    return strout;
}

// Internal methods
void string::updatePtr(void* ptr)
{
    m_ptr = ptr;
    m_ptrC = (char*)ptr;
}
//
void string::fixend(void)
{
    ((char*)m_ptr)[m_size] = '\0';
}
