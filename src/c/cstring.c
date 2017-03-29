#include <c/string.h>
#include <drivers/memory.h>

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

    char* strptr = (char*)alloc(length + 1);

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

char* strcenter(const char* const str, const size_t width, const size_t height)
{
    size_t strlength = strlen(str);

    size_t strwidth = (strlength > width ? width : strlength);
    size_t strheight = (strwidth / width) + !!(strwidth % width);

    size_t xoffset = (width - strwidth) / 2;
    size_t yoffset = (height > strheight ? (height - strheight) / 2 : 0);

    char* strout = (char*)alloc(yoffset + xoffset + strlength + 1);
    size_t istr = 0;

    for (size_t i = 0; i < yoffset; i++)
    {
        strout[istr++] = '\n';
    }

    for (size_t i = 0; i < xoffset; i++)
    {
        strout[istr++] = ' ';
    }

    // I was too lazy to figure out input with line breaks so I just replace them with spaces
    for (size_t i = 0; i < strlength; i++)
    {
        if (str[i] == '\n')
            strout[istr++] = ' ';
        else
            strout[istr++] = str[i];
    }

    strout[istr] = '\0';

    return strout;
}
