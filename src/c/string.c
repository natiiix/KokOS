#include <c/string.h>
#include <drivers/memory.h>
#include <drivers/io/terminal.h>
#include <kernel.h>

size_t strlen(const char* const strinput)
{
    size_t length = 0;

    while (strinput[length++]);

    return --length;
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

    char* strptr = (char*)mem_dynalloc(length + 1);

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
        debug_print("string.c | strparse() | Base out of range!");
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

char* strjoin(const char* const str1, const char* const str2)
{
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);

    char* strout = (char*)mem_dynalloc(len1 + len2 + 1);
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

    debug_print("string.c | strfirst() | String doesn't contain specified character!");
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

    debug_print("string.c | strlast() | String doesn't contain specified character!");
    return ~((size_t)0);
}

char* strfill(const char c, const size_t count)
{
    char* strFillChars = (char*)mem_dynalloc(count + 1);
    strFillChars[count] = '\0';
    
    for (size_t i = 0; i < count; i++)
    {
        strFillChars[i] = c;
    }

    return strFillChars;
}

char* colorToStr(const uint8_t color)
{
    char* strColor = (char*)mem_dynalloc(32);

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
