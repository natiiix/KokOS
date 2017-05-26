#include <modules/exec.hpp>
#include <c/stdio.h>
#include <c/math.h>

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
                Program::errorTypeUnexpected();
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
                Program::errorTypeUnexpected();
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

bool Program::symbolToReal(const string& strSymbol, REAL* const output, const bool throwError)
{
    // First check if the symbol is a variable name
    Variable* varSource = Program::varFind(strSymbol);

    // If the symbol represents an existing variable
    if (varSource)
    {
        // And the variable has the desired data type
        if (varSource->Type == DataType::Real)
        {
            // Set the output to the value of the variable
            (*output) = varSource->getReal();

            // Symbol represented a real variable and the value has been copied successfully
            return true;
        }
        // Symbol represents a variable of different data type
        else
        {
            if (throwError)
            {
                Program::errorTypeUnexpected();
            }

            return false;
        }
    }
    // The symbol could also be a literal value
    else
    {
        // Attempt to parse a real value from the symbol
        bool validLiteral = strSymbol.parseDouble(output);

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
        if (valueSource2 == 0)
        {
            Program::errorDivisionByZero();
            return false;
        }

        (*output) = (valueSource1 / valueSource2);
    }
    // Remainder after integer division
    else if (strOperator.compare("%"))
    {
        if (valueSource2 == 0)
        {
            Program::errorDivisionByZero();
            return false;
        }

        (*output) = (valueSource1 % valueSource2);
    }
    // Bit shift left
    else if (strOperator.compare("<<"))
    {
        (*output) = (valueSource1 << valueSource2);
    }
    // Bit shift right
    else if (strOperator.compare(">>"))
    {
        (*output) = (valueSource1 >> valueSource2);
    }
    // Bitwise AND
    else if (strOperator.compare("&"))
    {
        (*output) = (valueSource1 & valueSource2);
    }
    // Bitwise OR
    else if (strOperator.compare("|"))
    {
        (*output) = (valueSource1 | valueSource2);
    }
    // Bitwise XOR
    else if (strOperator.compare("^"))
    {
        (*output) = (valueSource1 ^ valueSource2);
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
    LOGICAL valueLogical1 = false;
    LOGICAL valueLogical2 = false;
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

    // Check input symbols for real values
    REAL valueReal1 = 0.0;
    REAL valueReal2 = 0.0;
    bool source1Real = false;
    bool source2Real = false;

    source1Real = Program::symbolToReal(strSymbol1, &valueReal1, false);
    source2Real = Program::symbolToReal(strSymbol2, &valueReal2, false);
    
    // Both source symbols represent valid real values
    if (source1Real && source2Real)
    {
        // Equality
        if (strOperator.compare("=="))
        {
            (*output) = (valueReal1 == valueReal2);
        }
        // Inequality
        else if (strOperator.compare("!="))
        {
            (*output) = (valueReal1 != valueReal2);
        }
        // Greater than
        else if (strOperator.compare(">"))
        {
            (*output) = (valueReal1 > valueReal2);
        }
        // Less than
        else if (strOperator.compare("<"))
        {
            (*output) = (valueReal1 < valueReal2);
        }
        // Greater than or equal to
        else if (strOperator.compare(">="))
        {
            (*output) = (valueReal1 >= valueReal2);
        }
        // Less than or equal to
        else if (strOperator.compare("<="))
        {
            (*output) = (valueReal1 <= valueReal2);
        }
        else
        {
            Program::errorOperatorInvalid(strOperator);
            return false;
        }

        return true;
    }

    // Failed to resolve the first source symbol
    if (!source1Logical && !source1Integer && !source1Real)
    {
        Program::errorSymbolUnresolved(strSymbol1);
    }
    // Failed to resolve the second source symbol
    else if (!source2Logical && !source2Integer && !source2Real)
    {
        Program::errorSymbolUnresolved(strSymbol2);
    }
    // Data types of source symbols do not match
    else
    {
        Program::errorTypeUnexpected();
    }

    return false;
}

bool Program::evaluateReal(const string& strSymbol1, const string& strOperator, const string& strSymbol2, REAL* const output)
{
    REAL valueSource1 = 0;
    REAL valueSource2 = 0;

    // Resolve both source values
    if (!Program::symbolToReal(strSymbol1, &valueSource1) ||
        !Program::symbolToReal(strSymbol2, &valueSource2))
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
    // Division
    else if (strOperator.compare("/"))
    {
        if (valueSource2 == 0.0)
        {
            Program::errorDivisionByZero();
            return false;
        }

        (*output) = (valueSource1 / valueSource2);
    }
    else
    {
        Program::errorOperatorInvalid(strOperator);
        return false;
    }

    return true;
}

bool Program::realSqrt(const string& strSourceSymbol, Variable* const outputVariable)
{
    REAL sourceValue = 0.0;

    // Resolve the source symbol
    if (!Program::symbolToReal(strSourceSymbol, &sourceValue))
    {
        // Source symbol is not a valid real value
        Program::errorSymbolUnresolved(strSourceSymbol);
        return false;
    }

    // Check for negative source value
    if (sourceValue < 0.0)
    {
        Program::error("Cannot perform square root on negative value!");
        return false;
    }

    // Set the variable to the result of the square root of the source value
    outputVariable->set(sqrt(sourceValue));
    return true;
}

bool Program::convertToInteger(const string& strSourceSymbol, Variable* const outputVariable)
{
    LOGICAL sourceLogical = false;
    REAL sourceReal = 0.0;

    // Test source symbol for logical value
    if (Program::symbolToLogical(strSourceSymbol, &sourceLogical, false))
    {
        // Source symbol is logical value
        // Convert the logical value to an integer value and store it in the output variable
        // false = 0; true = 1
        outputVariable->set((INTEGER)sourceLogical);
        return true;
    }
    // Test source symbol for real value
    else if (Program::symbolToReal(strSourceSymbol, &sourceReal, false))
    {
        // Source symbol is real value
        // Convert the real value to an integer value and store it in the output variable
        outputVariable->set((INTEGER)sourceReal);
        return true;
    }
    // Unable to resolve the source symbol
    else
    {
        Program::errorSymbolUnresolved(strSourceSymbol);
        return false;
    }
}

bool Program::convertToLogical(const string& strSourceSymbol, Variable* const outputVariable)
{
    INTEGER sourceInteger = 0;
    REAL sourceReal = 0.0;

    // Test source symbol for integer value
    if (Program::symbolToInteger(strSourceSymbol, &sourceInteger, false))
    {
        // Source symbol is integer value
        // Convert the integer to a logical value and store it in the output variable
        // 0 = false; else true
        outputVariable->set((LOGICAL)sourceInteger);
        return true;
    }
    // Test source symbol for real value
    else if (Program::symbolToReal(strSourceSymbol, &sourceReal, false))
    {
        // Source symbol is real value
        // Convert the real value to a logical value and store it in the output variable
        outputVariable->set((LOGICAL)sourceReal);
        return true;
    }
    // Unable to resolve the source symbol
    else
    {
        Program::errorSymbolUnresolved(strSourceSymbol);
        return false;
    }
}

bool Program::convertToReal(const string& strSourceSymbol, Variable* const outputVariable)
{
    INTEGER sourceInteger = 0;
    LOGICAL sourceLogical = false;

    // Test source symbol for integer value
    if (Program::symbolToInteger(strSourceSymbol, &sourceInteger, false))
    {
        // Source symbol is integer value
        // Convert the integer to a real value and store it in the output variable
        outputVariable->set((REAL)sourceInteger);
        return true;
    }
    // Test source symbol for logical value
    else if (Program::symbolToLogical(strSourceSymbol, &sourceLogical, false))
    {
        // Source symbol is logical value
        // Convert the logical value to a real value and store it in the output variable
        // false = 0.0; true = 1.0
        outputVariable->set((REAL)sourceLogical);
        return true;
    }
    // Unable to resolve the source symbol
    else
    {
        Program::errorSymbolUnresolved(strSourceSymbol);
        return false;
    }
}

bool Program::symbolToArrayInfo(const string& strSymbol, string& outName, size_t* const outAccessor)
{
    // Array syntax: <Name>@<Accessor>
    if (strSymbol.count('@') != 2)
    {
        return false;
    }

    // Split the symbol parts
    vector<string> vectParts = strSymbol.split('@', false);
    
    INTEGER iAccessor = 0;
    
    // Process the second part of the symbol into an accessor value
    if (!Program::symbolToInteger(vectParts.at(1), &iAccessor, true))
    {
        // Unable to process the accessor part
        vectParts.dispose();
        return false;
    }

    // The accessor can never be negative
    if (iAccessor < 0)
    {
        Program::error("Unexpected negative array accessor!");
        vectParts.dispose();
        return false;
    }

    // The first part of the symbol string (before the @ character) is the name of the array
    // Copy the array name to the output name string
    outName.set(vectParts.at(0));
    // Convert the accessor value to size_t and copy it to the output variable
    (*outAccessor) = (size_t)iAccessor;

    vectParts.dispose();
    return true;
}

void* Program::symbolToValue(const string& strSymbol, DataType* const outType)
{
    // Test symbol for being related to an array
    string arrayName;
    size_t arrayAccessor = 0;

    // If the symbol was successfully processed into array name and accessor
    if (Program::symbolToArrayInfo(strSymbol, arrayName, &arrayAccessor))
    {
        // Try to find an array with specified name
        Array* array = Program::arrayFind(arrayName);
        arrayName.dispose();
        
        // Array was successfully found
        if (array)
        {
            // Copy the array type to the output variable
            (*outType) = array->Type;
            // Return a pointer to the element specified by the accessor
            return Program::valueCopy(array->Type, array->getElementPtr(arrayAccessor));
        }
        // Array with specified name couldn't be found
        else
        {
            return nullptr;
        }        
    }

    arrayName.dispose();

    // Test symbol for being an array name
    Variable* var = Program::varFind(strSymbol);

    // Variable exists
    if (var)
    {
        // Copy variable data type to output variable
        (*outType) = var->Type;
        // Return a pointer to the value of the variable
        return Program::valueCopy(var->Type, var->Pointer);
    }

    // Test symbol for being a literal value
    INTEGER valueInteger = 0;
    LOGICAL valueLogical = false;
    REAL valueReal = 0.0;

    // If the literal value is successfully resolved
    // set the data type output variable to its data type,
    // clone the value of the literal into persistent
    // memory space and return a pointer to it

    // Symbol is an integer literal value
    if (strSymbol.parseInt32(&valueInteger))
    {
        (*outType) = DataType::Integer;
        return memstore(valueInteger);
    }
    // Symbol is a logical literal value
    else if (strSymbol.parseBool(&valueLogical))
    {
        (*outType) = DataType::Logical;
        return memstore(valueLogical);
    }
    // Symbol is a real literal value
    else if (strSymbol.parseDouble(&valueReal))
    {
        (*outType) = DataType::Real;
        return memstore(valueReal);
    }

    // Unable to resolve the symbol
    return nullptr;
}

void* Program::symbolMultiResolve(const vector<string> vectSymbols, const size_t firstIndex, DataType* const outType)
{
    size_t vectsize = vectSymbols.size();
    
    // There must be at least one symbol to resolve
    if (firstIndex >= vectsize)
    {
        return nullptr;
    }

    // Calculate the number of symbols to process
    size_t symbolCount = vectsize - firstIndex;

    // Single symbol can be resolved very easily
    if (symbolCount == 1)
    {
        // Convert the symbol to value and return the pointer to it and its data type
        return Program::symbolToValue(vectSymbols.at(firstIndex), outType);
    }

    // Operation on a single symbol
    if (symbolCount == 2)
    {
        DataType type;
        void* source = Program::symbolToValue(vectSymbols.at(firstIndex + 1), &type);

        // The input symbol is invalid
        if (!source)
        {
            return nullptr;
        }

        // Perform square root on the second symbol
        // Syntax: <Operation> <Input Symbol>
        if (vectSymbols.at(firstIndex).compare("sqrt"))
        {
            REAL realSourceValue = 0.0;

            // The source value already has real data type
            if (type == DataType::Real)
            {
                realSourceValue = *(REAL*)source;
            }
            // The source value doesn't have a real data type
            // Convert it to it from whatever data type it has
            else
            {
                realSourceValue = Program::toReal(source, type);
            }

            free(source);

            // Result of square root has always a real data type
            (*outType) = DataType::Real;
            // Perform the square root and store the result value in persistent memory and return a pointer to it
            return memstore(sqrt(realSourceValue));
        }

        free(source);
    }

    // Operation with two input symbols
    // Syntax: <First Symbol> <Operator> <Second Symbol>
    if (symbolCount == 3)
    {
        // Resolve the first input symbol
        DataType type1;
        void* source1 = Program::symbolToValue(vectSymbols.at(firstIndex), &type1);

        // The first input symbol is invalid
        if (!source1)
        {
            return nullptr;
        }

        // Resolve the second input symbol
        DataType type2;
        void* source2 = Program::symbolToValue(vectSymbols.at(firstIndex + 2), &type2);

        // The second input symbol is invalid
        if (!source2)
        {
            free(source1);
            return nullptr;
        }

        // If the input data have different data types perform a conversion
        // to the data type with the highest priority (real > integer > logical)
        if (type1 != type2)
        {

        }

        free(source1);
        free(source2);
    }

    // Invalid number of symbols
    return nullptr;
}
