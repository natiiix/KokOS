#include "class_string.hpp"

#include "cstring.hpp"
#include "memory.hpp"
#include "class_vector.hpp"

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
//
void string::pop_back(const size_t popcount)
{
	if (popcount > 0)
	{
		string::resize(m_size - popcount);
	}
}

// String operations
char* string::c_str(void)
{
    return m_ptrC;
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
bool string::compare(const char* const str)
{
    // Check the whole string including the ending '\0'
    for (size_t i = 0; i < m_size + 1; i++)
    {
        if (str[i] != m_ptrC[i])
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
//
vector<string> string::split(const char cDelimiter, const bool removeEmpty)
{
	vector<string> vectout;
	size_t partstart = 0;
	
	for (size_t i = 0; i < m_size; i++)
	{
		if (m_ptrC[i] == cDelimiter)
		{
			splitVectorAdd(vectout, partstart, i, removeEmpty);
			partstart = i + 1;
		}
	}
	
	splitVectorAdd(vectout, partstart, m_size, removeEmpty);
	
	return vectout;
}
//
vector<string> string::split(const char* const strDelimiter, const bool removeEmpty)
{
	vector<string> vectout;
	size_t delimlen = cstr::len(strDelimiter);
	
	if (delimlen == 0)
	{
		vectout.push_back(string(m_ptrC));
		return vectout;
	}
	
	size_t partstart = 0;
	bool delimfound = false;
	
	size_t i = 0;
	while (i < m_size)
	{
		if (m_ptrC[i] == strDelimiter[0])
		{
			delimfound = true;
			
			for (size_t j = 1; j < delimlen; j++)
			{
				if (m_ptrC[i + j] != strDelimiter[j])
				{
					delimfound = false;
					break;
				}
			}
			
		}
		
		if (delimfound)
		{
			splitVectorAdd(vectout, partstart, i, removeEmpty);
			partstart = (i += delimlen);
			delimfound = false;
		}
		else
		{
			i++;
		}
	}
	
	splitVectorAdd(vectout, partstart, m_size, removeEmpty);
	
	return vectout;
}

// Operator overloads
bool string::operator==(const string& str)
{
    return string::compare(str);
}
//
bool string::operator==(const char* const str)
{
    return string::compare(str);
}
//
char& string::operator[](const size_t idx)
{
	return string::at(idx);
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
    m_ptrC[m_size] = '\0';
}
//
void string::splitVectorAdd(vector<string>& vectsplit, const size_t start, const size_t end, const bool removeEmpty)
{
	if (end < start)
	{
		return;
	}	
	else if (end == start)
	{
		if (removeEmpty)
		{
			return;
		}
		else
		{
			vectsplit.push_back(string());
		}
	}
	else
	{
		vectsplit.push_back(string::substr(start, end - start));
	}	
}
