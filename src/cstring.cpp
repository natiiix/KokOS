#include "cstring.hpp"

namespace cstr
{
    size_t len(const char* const strinput)
    {
        size_t length = 0;
        while (strinput[length])
            length++;
        return length;
    }

    bool cmp(const char* str1, const char* str2)
    {
        size_t length = len(str1);
        
        if (len(str2) != length)
            return false;

        for (size_t i = 0; i < length; i++)
        {
            if (str1[i] != str2[i])
                return false;
        }

        return true;
    }

    char* convert(const size_t input, size_t base)
    {
        if (base < 2 || base > 16)
            return nullptr;

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
                    digit += '0';
                else if (digit >= 10 && digit < 16)
                    digit += 'A' - 10;
                
                strvalue[length++] = digit;
            }
        }

        char* strptr = (char*)mem::alloc(length + 1);

        for (size_t i = 0; i < length; i++)
        {
            strptr[i] = strvalue[length - 1 - i];
        }

        strptr[length] = '\0';
        return strptr;
    }

    size_t parse(const char* const str, const size_t base)
    {
        if (base < 2 || base > 16)
            return 0;

        size_t value = 0;

        for (size_t i = 0; str[i]; i++)
        {
            value *= base;
            
            if (str[i] >= '0' && str[i] <= '9')
                value += str[i] - '0';
            else if (str[i] >= 'a' && str[i] <= 'f')
                value += str[i] - 'a' + 10;
            else if (str[i] >= 'A' && str[i] <= 'F')
                value += str[i] - 'A' + 10;
        }

        return value;
    }
}
