#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#include <c/stdlib.h>
#include <c/stdio.h>
#include <c/string.h>
#include <c/math.h>

#include <kernel.h>

string::string(void) :
    m_ptr(malloc(1)),
    m_ptrC((char*)m_ptr),
    m_size(0)
{
    fixend();
}

string::string(const char* const str)
{
    size_t strlength = strlen(str);
    
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

string string::copy(void)
{
    string strout;
    
    strout.clear();
    strout.push_back(m_ptrC);

    return strout;
}

void string::set(const char* const str)
{
    string::clear();
    string::push_back(str);
}

// Capacity
size_t string::size(void) const
{
    return m_size;
}
//
void string::resize(const size_t newsize)
{
    updatePtr(realloc(m_ptr, (m_size = newsize) + 1));
    fixend();
}
//
void string::clear(void)
{
    string::resize(0);
}
//
bool string::empty(void) const
{
    return !m_size;
}

// Element access
char string::at(const size_t idx) const
{
    if (idx <= m_size) // m_ptrC[m_size] is a string terminator
    {
        return m_ptrC[idx];
    }
    else
    {
        debug_print("string.cpp | string::at() | Index out of string boundaries!");
        return '\0';
    }
}
//
char& string::front(void)
{
    if (m_size > 0)
    {
        return m_ptrC[0];
    }
    else
    {
        debug_print("string.cpp | string::front() | String is empty!");
        return m_ptrC[m_size]; // return reference to the string terminator
    }
}
//
char& string::back(void)
{
    if (m_size > 0)
    {
        return m_ptrC[m_size - 1];
    }
    else
    {
        debug_print("string.cpp | string::back() | String is empty!");
        return m_ptrC[m_size]; // return reference to the string terminator
    }
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
    size_t strlength = strlen(str);
    
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
    if (m_size > 0)
    {
        string::resize(m_size - 1);
    }
    else
    {
        debug_print("string.cpp | string::pop_back() | String is empty!");
    }
}
//
void string::pop_back(const size_t popcount)
{
	if (popcount > 0)
	{
        if (popcount < m_size)
        {
		    string::resize(m_size - popcount);
        }
        else
        {
            debug_print("string.cpp | string::pop_back() | Pop count is higher than the string length!");
            string::resize(0);
        }
	}
    else
    {
        debug_print("string.cpp | string::pop_back() | String is empty!");
    }
}

// String operations
char* string::c_str(void) const
{
    return m_ptrC;
}
//
string string::substr(const size_t pos) const
{
    return string::substr(pos, m_size - pos);
}
//
string string::substr(const size_t pos, const size_t len) const
{
    string strout;

    if (pos >= m_size)
    {
        debug_print("string.cpp | string::substr() | Position index is out of string boundaries!");
        return strout;
    }

    // If len is higher than the number of remaining
    // characters the rest of the string is returned
    size_t copylen = len;
    size_t remainchar = m_size - pos;

    if (len > remainchar)
    {
        debug_print("string.cpp | string::substr() | Length is higher than the remaining size of the string!");
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
bool string::compare(const string& str) const
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
bool string::compare(const char* const str) const
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
string string::tolower(void) const
{
    string strout;

    for (size_t i = 0; i < m_size; i++)
    {
        strout.push_back(ctolower(m_ptrC[i]));
    }

    return strout;
}
//
string string::toupper(void) const
{
    string strout;

    for (size_t i = 0; i < m_size; i++)
    {
        strout.push_back(ctoupper(m_ptrC[i]));
    }

    return strout;
}
//
vector<string> string::split(const char cDelimiter, const bool removeEmpty) const
{
	vector<string> vectout;
    vectout.clear();
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
vector<string> string::split(const char* const strDelimiter, const bool removeEmpty) const
{
	vector<string> vectout;
	size_t delimlen = strlen(strDelimiter);
	
	if (delimlen == 0)
	{
        debug_print("string.cpp | string::split() | Delimiter string is empty!");
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
//
bool string::contains(const char* const str) const
{
    size_t strlength = strlen(str);

    for (size_t i = 0; i < m_size - strlength + 1; i++)
    {
        if (m_ptrC[i] == str[0])
        {
            bool match = true;

            for (size_t j = 1; j < strlength; j++)
            {
                if (m_ptrC[i + j] != str[j])
                {
                    match = false;
                }
            }

            if (match)
            {
                return true;
            }
        }
    }

    return false;
}
//
void string::remove(const size_t pos, const size_t len)
{
    shiftCharsLeft(pos, len);
}
//
void string::insert(const char c, const size_t pos)
{
    shiftCharsRight(pos, 1);

    m_ptrC[pos] = c;
}
//
void string::insert(const string& str, const size_t pos)
{
    size_t strsize = str.size();
    shiftCharsRight(pos, strsize);

    for (size_t i = 0; i < strsize; i++)
    {
        m_ptrC[pos + i] = str.at(i);
    }
}
//
bool string::parseInt32(int32_t* const output) const
{
    // Can't parse a value from an empty string
    if (!m_size)
    {
        // Parsing failed
        return false;
    }

    // Check for value negativity based on the first character being a minus sign
    bool negative = (string::at(0) == '-');
    
    int32_t value = 0;

    // Ignore the initial minus sign if the value is negative
    for (size_t i = negative; i < m_size; i++)
    {
        char cDigit = string::at(i);
        
        // Check if the character is a valid digit
        if (cDigit >= '0' && cDigit <= '9')
        {
            int32_t digitWeight = powInt32(10, m_size - 1 - i);
            value += (cDigit - '0') * digitWeight;
        }
        // Invalid character occurred
        else
        {
            // Parsing failed
            return false;
        }
    }

    // Value parsed, now just solve negativity
    if (negative)
    {
        (*output) = -value;
    }
    else
    {
        (*output) = value;
    }

    // Value has been parse successfully
    return true;
}
//
bool string::parseBool(bool* const output) const
{
    if (string::compare("false"))
    {
        (*output) = false;
        return true;
    }
    else if (string::compare("true"))
    {
        (*output) = true;
        return true;
    }
    else
    {
        return false;
    }
}
//
bool string::parseDouble(double* const output) const
{
    // String is empty, therefore it cannot contain a valid double value
    if (!m_size)
    {
        return false;
    }

    bool negative = false;
    bool dotFound = false;
    size_t dotIdx = 0;

    char digits[256];
    size_t digitCount = 0;

    // Check if string contains a valid double value
    for (size_t i = 0; i < m_size; i++)
    {
        // Check the first character in the string for minus sign
        if (!i && m_ptrC[0] == '-')
        {
            negative = true;
        }
        // Decimal dot reached
        else if (!dotFound && m_ptrC[i] == '.')
        {
            // There is no digit before the decimal dot
            // Add an artificial 0 to the beginning as if it were there
            if (!digitCount)
            {
                digits[digitCount++] = '0';
                dotIdx = 1;
            }
            else
            {
                dotIdx = i;
            }

            dotFound = true;
        }
        // Valid digit character
        else if (m_ptrC[i] >= '0' && m_ptrC[i] <= '9')
        {
            digits[digitCount++] = m_ptrC[i];
        }
        // Unexpected character found
        else
        {
            return false;
        }
    }

    // If there is no decimal dot assume the number is integer
    if (!dotFound)
    {
        dotIdx = digitCount;
    }

    double value = 0.0;
    int32_t exponent = dotIdx - 1;

    // Translate digits into the actual double value
    for (size_t i = 0; i < digitCount; i++)
    {
        if (digits[i] != '0')
        {
            // Convert the digit character to its value
            double digitValue = ((double)(digits[i] - '0')) * powDouble(10.0, exponent);
            double tmpValue = value + digitValue;

            // The precision limit has been hit
            // Further digits wouldn't change the output value due to their insignificance
            if (tmpValue == value)
            {
                break;
            }
            // Digit is significant enough
            else
            {
                // Update the value
                value = tmpValue;
            }
        }
        
        // Decrement the exponent
        exponent--;
    }

    // Set the output variable to the parsed value
    (*output) = (negative ? -value : value);
    return true;
}

// Operator overloads
bool string::operator==(const string& str) const
{
    return string::compare(str);
}
//
bool string::operator==(const char* const str) const
{
    return string::compare(str);
}
//
char& string::operator[](const size_t idx)
{
	if (idx <= m_size) // m_ptrC[m_size] is a string terminator
    {
        return m_ptrC[idx];
    }
    else
    {
        debug_print("string.cpp | string::operator[]() | Index out of string boundaries!");
        return m_ptrC[m_size]; // return reference to the string terminator
    }
}
//
string& string::operator+=(const string& str)
{
    string::push_back(str);
    return *this;
}
//
string& string::operator+=(const char c)
{
    string::push_back(c);
    return *this;
}
//
string& string::operator+=(const char* const str)
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

string string::join(const vector<string>& vect, const char cDelimiter, const bool removeEmpty)
{
    string strout;
    strout.clear();

    size_t vectsize = vect.size();

    for (size_t i = 0; i < vectsize; i++)
    {
        if (!removeEmpty || vect.at(i).size() > 0)
        {
            if (strout.size() > 0)
            {
                strout.push_back(cDelimiter);
            }

            strout.push_back(vect.at(i));
        }
    }

    return strout;
}

string string::toString(const int32_t value)
{
    string strout;
    strout.clear();

    int32_t tmp = value;

    char digits[32];
    size_t digitCount = 0;

    if (tmp < 0)
    {
        strout.push_back('-');
        tmp = -tmp;
    }

    do
    {
        digits[digitCount++] = '0' + (tmp % 10);
        tmp /= 10;
    }
    while (tmp);

    for (size_t i = 0; i < digitCount; i++)
    {
        strout.push_back(digits[digitCount - 1 - i]);
    }

    return strout;
}
//
string string::toString(const bool value)
{
    string strout;
    strout.clear();

    if (value)
    {
        strout.push_back("true");
    }
    else
    {
        strout.push_back("false");
    }

    return strout;
}
//
string string::toString(const double value)
{
    string strout;
    strout.clear();

    // The value is exactly zero
    if (value == 0.0)
    {
        strout.push_back("0.0");
        return strout;
    }

    // Put minus sign at the beginning of the string if the value is negative
    if (value < 0.0)
    {
        strout.push_back('-');
    }

    // Get the absolute value of the input value
    double absValue = absDouble(value);

    // Determine how many digits are there before the decimal dot
    // If the value is lower than 1 get the number of zeros to add before the first non-zero digit
    int32_t digitsBeforeDot = 0;

    // There is at least 1 digit before the decimal dot
    if (absValue >= 1.0)
    {
        // Keep dividing the value by 10 until it has no digits before the decimal dot
        while (absValue >= 1.0)
        {
            digitsBeforeDot++;
            absValue /= 10;
        }
    }
    // All of the digits are after the decimal dot
    else if (absValue > 0.0)
    {
        // Keep multiplying the value by 10 until it's high enough to have a digit right after the dot
        while (absValue < 0.1)
        {
            digitsBeforeDot--;
            absValue *= 10;
        }
    }

    // Write leading zeros
    if (digitsBeforeDot <= 0)
    {
        // Write the zero at the units position
        strout.push_back("0.");

        // Write all the decimal zeros
        while (digitsBeforeDot < 0)
        {
            digitsBeforeDot++;
            strout.push_back('0');
        }
    }

    size_t zeroChain = 0;
    size_t nineChain = 0;
    static const size_t chainLimit = 8;

    size_t decimalCount = 0;
    static const size_t decimalLimit = 20;

    // Write the rest of the digits
    while ((absValue > 0.0 && decimalCount < decimalLimit) || digitsBeforeDot > 0)
    {
        // Compute the value of this digit
        absValue *= 10;
        uint8_t digitValue = 0;

        while (absValue > 1.0)
        {
            absValue -= 1.0;
            digitValue++;
        }

        // Look out for repeating decimal digits
        if (digitsBeforeDot == 0)
        {
            // Chain of zeros
            if (digitValue == 0)
            {
                zeroChain++;
                nineChain = 0;

                // Chain length limit reached
                if (zeroChain == chainLimit)
                {
                    // Pop all the redundant zeros
                    strout.pop_back(chainLimit - 1);
                    break;
                }
            }
            // Chain of nines
            else if (digitValue == 9)
            {
                nineChain++;
                zeroChain = 0;

                // Chain length limit reached
                if (nineChain == chainLimit)
                {
                    // Can't increment the pre-chain digit if the whole decimal part is the chain
                    if (strout.at(strout.size() - chainLimit) != '.')
                    {
                        // Pop all the redundant nines
                        strout.pop_back(chainLimit - 1);
                        // Increment the last digit before the sequence of nines
                        strout.back()++;
                    }

                    break;
                }
            }
            else
            {
                zeroChain = 0;
                nineChain = 0;
            }
            
            decimalCount++;
        }
        
        // Ignore zero at the end of the the decimal part
        if (decimalCount != decimalLimit || digitValue != 0)
        {
            // Push the digit character to the string
            strout.push_back('0' + digitValue);
        }

        // If we're not in the decimal part yet
        if (digitsBeforeDot > 0)
        {
            // Decrease the number of remaining whole part digits
            digitsBeforeDot--;

            // If this was the last whole part digit
            if (digitsBeforeDot == 0)
            {
                // Push the decimal dot to the string
                strout.push_back('.');

                // If the value is an integer put a zero at the end of the string
                if (absValue == 0.0)
                {
                    strout.push_back('0');
                }
            }
        }
    }

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
void string::splitVectorAdd(vector<string>& vectsplit, const size_t start, const size_t end, const bool removeEmpty) const
{
	if (end < start)
	{
        debug_print("string.cpp | string::splitVectorAdd() | Start index is higher than end index!");
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
            // The string object must be constructed
            string strEmpty;
			vectsplit.push_back(strEmpty);
		}
	}
	else
	{
        vectsplit.push_back(string::substr(start, end - start));
    }
}
//
void string::shiftCharsRight(const size_t pos, const size_t offset)
{
    if (pos >= m_size)
    {
        debug_print("string.cpp | string::shiftCharsRight() | Position index is out of string boundaries!");
        return;
    }

    size_t oldsize = m_size;
    resize(m_size + offset);

    size_t shiftedElements = oldsize - pos;

    for (size_t i = 0; i < shiftedElements; i++)
    {
        m_ptrC[m_size - 1 - i] = m_ptrC[oldsize - 1 - i];
    }
}
//
void string::shiftCharsLeft(const size_t pos, const size_t offset)
{
    if (pos >= m_size || m_size - pos < offset)
    {
        debug_print("string.cpp | string::shiftCharsLeft() | Can't shift character out of the string!");
        return;
    }
    
    for (size_t i = pos; i < m_size; i++)
    {
        m_ptrC[i] = m_ptrC[i + offset];
    }

    resize(m_size - offset);
}

void sprint(const string& str)
{
    print(str.c_str());
}

void sprintat(const string& str, const size_t col, const size_t row)
{
    printat(str.c_str(), col, row);
}
