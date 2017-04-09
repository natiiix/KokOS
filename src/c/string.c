#include <c/string.h>
#include <drivers/memory.h>
#include <drivers/io/terminal.h>

size_t strlen(const char* const strinput)
{
    size_t length = 0;
    
    while (strinput[length])
    {
        length++;
    }

    return length;
}

bool strcmp(const char* str1, const char* str2)
{
    size_t length = strlen(str1);
    
    if (strlen(str2) != length)
    {
        return false;
    }

    for (size_t i = 0; i < length; i++)
    {
        if (str1[i] != str2[i])
        {
            return false;
        }
    }

    return true;
}

char* tostr(const size_t input, const size_t base)
{
    if (base < 2 || base > 16)
    {
        return (char*)0;
    }

    // 65 is the maximum length of a string representation of a numeric value
    // which would be 64 bit integer in base 2 including the ending character '\0'
    const size_t maxlength = 65;
    char strvalue[maxlength];
    size_t length = 0;
    char digit = 0;

    if (input == 0)
    {
        strvalue[length++] = '0';
    }
    else
    {
        for (size_t remainder = input; remainder > 0; remainder /= base)
        {
            digit = (char)(remainder % base);
            
            if (digit >= 0 && digit < 10)
            {
                digit += '0';
            }
            else if (digit >= 10 && digit < 16)
            {
                digit += 'A' - 10;
            }
            
            strvalue[length++] = digit;
        }
    }

    char* strptr = (char*)mem_alloc(length + 1);

    for (size_t i = 0; i < length; i++)
    {
        strptr[i] = strvalue[length - 1 - i];
    }

    strptr[length] = '\0';
    return strptr;
}

size_t strparse(const char* const str, const size_t base)
{
    if (base < 2 || base > 16)
    {
        return 0;
    }

    size_t value = 0;

    for (size_t i = 0; str[i]; i++)
    {
        value *= base;
        
        if (str[i] >= '0' && str[i] <= '9')
        {
            value += str[i] - '0';
        }
        else if (str[i] >= 'a' && str[i] <= 'f')
        {
            value += str[i] - 'a' + 10;
        }
        else if (str[i] >= 'A' && str[i] <= 'F')
        {
            value += str[i] - 'A' + 10;
        }
    }

    return value;
}

char* strcenter(const char* const str)
{
    size_t len = strlen(str);

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

        if (str[i] == '\n')
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

    char* strbuff = (char*)mem_alloc(VGA_HEIGHT * VGA_WIDTH);
    size_t buffIdx = 0;

    size_t marginTop = (VGA_HEIGHT / 2) - (linesCount / 2);
    for (size_t i = 0; i < marginTop; i++)
    {
        strbuff[buffIdx++] = '\n';
    }

    for (size_t i = 0; i < linesCount; i++)
    {
        size_t marginLeft = (VGA_WIDTH / 2) - (linesLen[i] / 2) - (linesLen[i] % 2);
        for (size_t j = 0; j < marginLeft; j++)
        {
            strbuff[buffIdx++] = ' ';
        }

        for (size_t j = 0; j < linesLen[i]; j++)
        {
            strbuff[buffIdx++] = str[linesOffset[i] + j];
        }

        if (i < linesCount - 1 && linesLen[i] < VGA_WIDTH)
        {
            strbuff[buffIdx++] = '\n';
        }
    }

    strbuff[buffIdx] = '\0';

    return strbuff;
}

char* strjoin(const char* const str1, const char* const str2)
{
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);

    char* strout = (char*)mem_alloc(len1 + len2 + 1);
    size_t strIdx = 0;

    for (size_t i = 0; i < len1; i++)
    {
        strout[strIdx++] = str1[i];
    }

    for (size_t i = 0; i < len2; i++)
    {
        strout[strIdx++] = str2[i];
    }

    strout[strIdx] = '\0';

    return strout;
}

void strcopy(const char* const strInput, char* const strOutput)
{
    size_t len = strlen(strInput);

    for (size_t i = 0; i < len; i++)
    {
        strOutput[i] = strInput[i];
    }

    strOutput[len] = '\0';
}

char ctoupper(const char c)
{
    if (c >= 'a' && c <= 'z')
    {
        return 'A' + (c - 'a');
    }

    return c;
}

char ctolower(const char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return 'a' + (c - 'A');
    }

    return c;
}

void strtoupper(char* const str)
{
    for (size_t i = 0; str[i]; i++)
    {
        str[i] = ctoupper(str[i]);
    }
}

void strtolower(char* const str)
{
    for (size_t i = 0; str[i]; i++)
    {
        str[i] = ctolower(str[i]);
    }
}

size_t strfirst(const char* const str, const char c)
{
    size_t strsize = strlen(str);

    for (size_t i = 0; i < strsize; i++)
    {
        if (str[i] == c)
        {
            return i;
        }
    }

    return ~((size_t)0);
}

size_t strlast(const char* const str, const char c)
{
    size_t strsize = strlen(str);

    for (size_t i = 0; i < strsize; i++)
    {
        if (str[strsize - 1 - i] == c)
        {
            return strsize - 1 - i;
        }
    }

    return ~((size_t)0);
}
