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

double sqrt(const double value)
{
    // Negative input value
	if (value < 0.0)
	{
		// Return arbitrary negative value as a sign of error
		return -1.0;
	}
	// Input value is exactly zero
	else if (value == 0.0)
	{
		// Square root of zero is zero
		return 0.0;
	}
	// Input value is exactly one
	else if (value == 1.0)
	{
		// Square root of one is one
		return 1.0;
	}

	double sqrtValue = 0.0; // result storage
	int32_t exponent = 0; // currently used power of two

	// Determine how which power of two is the largest lower value than the input value
	if (value > 1.0)
	{
		double tmp = value;

		while (true)
		{
			tmp /= 2.0;

			if (tmp > 1.0)
			{
				exponent++;
			}
			else
			{
				break;
			}
		}
	}

	// Until the square root is found
	while (sqrtValue * sqrtValue < value)
	{
		// Get the currenly worked with power of two
		double incValue = powDouble(2.0, exponent);
		// Add it to the already computed square root
		double tmpValue = sqrtValue + incValue;

		// Precision limit reached
		// Adding any lower powers of two won't make a change
		if (tmpValue == sqrtValue)
		{
			break;
		}

		// Check whether this power of two should be added or not
		if (tmpValue * tmpValue <= value)
		{
			sqrtValue = tmpValue;
		}

		// Decrease the exponent for the next iteration
		exponent--;
	}

	return sqrtValue;
}
