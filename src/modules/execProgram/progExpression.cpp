#include <modules/exec.hpp>
#include <c/stdio.h>

bool Program::symbolToInteger(const string& strSymbol, INTEGER* const output, const bool throwError)
{
    // First check if the symbol is a variable name
    Variable* varSource = Program::varFind(strSymbol);

    // If the symbol represents an existing variable
    if (varSource)
    {
        // And the variable has the desired data type
        if (varSource->Type == DataType::Integer)
        {
            // Set the output to the value of the variable
            (*output) = varSource->getInteger();

            // Symbol represented an integer variable and the value has been copied successfully
            return true;
        }
        // Symbol represents a variable of different data type
        else
        {
            if (throwError)
            {
                Program::errorTypesIncompatible();
            }

            return false;
        }
    }
    // The symbol could also be a literal value
    else
    {
        // Attempt to parse an integer value from the symbol
        bool validLiteral = strSymbol.parseInt32(output);

        // Symbol parsing failed
        if (!validLiteral && throwError)
        {
            Program::errorSymbolUnresolved(strSymbol);
        }

        return validLiteral;
    }
}

bool Program::symbolToLogical(const string& strSymbol, LOGICAL* const output, const bool throwError)
{
    // First check if the symbol is a variable name
    Variable* varSource = Program::varFind(strSymbol);

    // If the symbol represents an existing variable
    if (varSource)
    {
        // And the variable has the desired data type
        if (varSource->Type == DataType::Logical)
        {
            // Set the output to the value of the variable
            (*output) = varSource->getLogical();

            // Symbol represented a logical variable and the value has been copied successfully
            return true;
        }
        // Symbol represents a variable of different data type
        else
        {
            if (throwError)
            {
                Program::errorTypesIncompatible();
            }

            return false;
        }
    }
    // The symbol could also be a literal value
    else
    {
        // Attempt to parse a logical value from the symbol
        bool validLiteral = strSymbol.parseBool(output);

        // Symbol parsing failed
        if (!validLiteral && throwError)
        {
            Program::errorSymbolUnresolved(strSymbol);
        }

        return validLiteral;
    }
}

bool Program::evaluateInteger(const string& strSymbol1, const string& strOperator, const string& strSymbol2, INTEGER* const output)
{
    INTEGER valueSource1 = 0;
    INTEGER valueSource2 = 0;

    // Resolve both source values
    if (!Program::symbolToInteger(strSymbol1, &valueSource1) ||
        !Program::symbolToInteger(strSymbol2, &valueSource2))
    {
        // Unable to resolve one of the symbols
        return false;
    }

    // Addition
    if (strOperator.compare("+"))
    {
        (*output) = (valueSource1 + valueSource2);
    }
    // Subtraction
    else if (strOperator.compare("-"))
    {
        (*output) = (valueSource1 - valueSource2);
    }
    // Multiplication
    else if (strOperator.compare("*"))
    {
        (*output) = (valueSource1 * valueSource2);
    }
    // Integer division
    else if (strOperator.compare("/"))
    {
        (*output) = (valueSource1 / valueSource2);
    }
    // Remainder after integer division
    else if (strOperator.compare("%"))
    {
        (*output) = (valueSource1 % valueSource2);
    }
    else
    {
        Program::errorOperatorInvalid(strOperator);
        return false;
    }

    return true;
}

bool Program::evaluateLogical(const string& strSymbol1, const string& strOperator, const string& strSymbol2, LOGICAL* const output)
{
    // Check input symbols for logical values
    LOGICAL valueLogical1 = 0;
    LOGICAL valueLogical2 = 0;
    bool source1Logical = false;
    bool source2Logical = false;

    source1Logical = Program::symbolToLogical(strSymbol1, &valueLogical1, false);
    source2Logical = Program::symbolToLogical(strSymbol2, &valueLogical2, false);

    // Both source symbols represent valid logical values
    if (source1Logical && source2Logical)
    {
        // Boolean NXOR (equality check)
        if (strOperator.compare("=="))
        {
            (*output) = (valueLogical1 == valueLogical2);
        }
        // Boolean XOR (inequality check)
        else if (strOperator.compare("!="))
        {
            (*output) = (valueLogical1 != valueLogical2);
        }
        // Boolean AND
        else if (strOperator.compare("&&"))
        {
            (*output) = (valueLogical1 && valueLogical2);
        }
        // Boolen OR
        else if (strOperator.compare("||"))
        {
            (*output) = (valueLogical1 || valueLogical2);
        }
        else
        {
            Program::errorOperatorInvalid(strOperator);
            return false;
        }

        return true;
    }

    // Check input symbols for integer values
    INTEGER valueInteger1 = 0;
    INTEGER valueInteger2 = 0;
    bool source1Integer = false;
    bool source2Integer = false;

    source1Integer = Program::symbolToInteger(strSymbol1, &valueInteger1, false);
    source2Integer = Program::symbolToInteger(strSymbol2, &valueInteger2, false);
    
    // Both source symbols represent valid integer values
    if (source1Integer && source2Integer)
    {
        // Equality
        if (strOperator.compare("=="))
        {
            (*output) = (valueInteger1 == valueInteger2);
        }
        // Inequality
        else if (strOperator.compare("!="))
        {
            (*output) = (valueInteger1 != valueInteger2);
        }
        // Greater than
        else if (strOperator.compare(">"))
        {
            (*output) = (valueInteger1 > valueInteger2);
        }
        // Less than
        else if (strOperator.compare("<"))
        {
            (*output) = (valueInteger1 < valueInteger2);
        }
        // Greater than or equal to
        else if (strOperator.compare(">="))
        {
            (*output) = (valueInteger1 >= valueInteger2);
        }
        // Less than or equal to
        else if (strOperator.compare("<="))
        {
            (*output) = (valueInteger1 <= valueInteger2);
        }
        else
        {
            Program::errorOperatorInvalid(strOperator);
            return false;
        }

        return true;
    }

    // Failed to resolve the first source symbol
    if (!source1Logical && !source1Integer)
    {
        Program::errorSymbolUnresolved(strSymbol1);
    }
    // Failed to resolve the second source symbol
    else if (!source2Logical && !source2Integer)
    {
        Program::errorSymbolUnresolved(strSymbol2);
    }
    // Data types of source symbols do not match
    else
    {
        Program::errorTypesIncompatible();
    }

    return false;
}