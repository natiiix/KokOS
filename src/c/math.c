#include <stddef.h>
#include <stdint.h>

int32_t powInt32(const int32_t value, const size_t power)
{
    int32_t output = 1;

    for (size_t i = 0; i < power; i++)
    {
        output *= value;
    }

    return output;
}

double powDouble(const double value, const size_t power)
{
    double output = 1;

    for (size_t i = 0; i < power; i++)
    {
        output *= value;
    }

    return output;
}