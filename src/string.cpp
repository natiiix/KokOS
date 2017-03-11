#include "string.hpp"

namespace str
{
    size_t len(const char* strinput)
    {
        size_t length = 0;
        while (strinput[length])
            length++;
        return length;
    }

    char* convert(const size_t input)
    {
        char strvalue[16];
        size_t length = 0;
        size_t remainder = input;

        while (remainder > 0)
        {
            strvalue[length++] = (char)(remainder % 10) + '0';
            remainder /= 10;
        }

        char* strptr = (char*)mem::alloc(length + 1);

        for (size_t i = 0; i < length; i++)
        {
            strptr[i] = strvalue[length - 1 - i];
        }

        strptr[length] = '\0';
        return strptr;
    }

    void convertat(const size_t input, char* output)
    {
        char strvalue[16];
        size_t length = 0;
        size_t remainder = input;

        while (remainder > 0)
        {
            strvalue[length++] = (char)(remainder % 10) + '0';
            remainder /= 10;
        }

        for (size_t i = 0; i < length; i++)
        {
            output[i] = strvalue[length - 1 - i];
        }

        output[length] = '\0';
    }
}
