#include <stddef.h>
#include <stdint.h>

#include <c/math.h>

int32_t powInt32(const int32_t value, const int32_t power)
{
    int32_t output = 1;

    size_t powAbs = absInt32(power);

    for (size_t i = 0; i < powAbs; i++)
    {
        output *= value;
    }

    // Positive / zero exponent
    if (power >= 0)
    {
        return output;
    }
    // Negative exponent
    else
    {
        return 1 / output;
    }
}

double powDouble(const double value, const int32_t power)
{
    double output = 1;

    size_t powAbs = absInt32(power);

    for (size_t i = 0; i < powAbs; i++)
    {
        output *= value;
    }    

    // Positive / zero exponent
    if (power >= 0)
    {
        return output;
    }
    // Negative exponent
    else
    {
        return 1 / output;
    }
}

int32_t absInt32(const int32_t value)
{
    return (value < 0 ? -value : value);
}

double absDouble(const double value)
{
    return (value < 0 ? -value : value);
}
