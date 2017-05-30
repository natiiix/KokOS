#include <modules/exec.hpp>
#include <c/stdio.h>

bool Program::varDeclare(const string& name, const DataType type)
{
    // Check variable name validity
    enum PROGRAM_NAME nameResult = Program::nameValid(name);
    if (nameResult == PROGRAM_NAME_OK)
    {
        // Push the new variable into the variable vector
        m_variables.push_back(Variable());
        m_variables.back().declare(name, type, m_scope);

        // Variable declared successfully
        return true;
    }
    else
    {
        string strError;
        strError.clear();

        strError.push_back("Variable name \"");
        strError.push_back(name);
        
        // Name contain invalid character
        if (nameResult == PROGRAM_NAME_INVALID_CHAR)
        {
            strError.push_back("\" contains an invalid character!\nValid characters: { a-z, A-Z, 0-9, _ }");
        }
        // Name is a keyword
        else if (nameResult == PROGRAM_NAME_KEYWORD)
        {
            strError.push_back("\" conflicts with a keyword!");
        }
        // Unknown name-related error occurred
        else
        {
            strError.push_back("\" is invalid for unknown reason!");
        }

        Program::error(strError);
        strError.dispose();

        // Failed to define a variable with specified name
        return false;
    }
}

Variable* Program::varFind(const string& name)
{
    size_t varsize = m_variables.size();
    
    for (size_t i = 0; i < varsize; i++)
    {
        // Variables are being searched from back to front to make sure that
        // when multiple variables share the same name (due to recursion for example)
        // the last declared variable with the name is returned
        size_t varIdx = varsize - 1 - i;

        // Find the variable with the specified name
        if (m_variables[varIdx].Name.compare(name))
        {
            // Return pointer to the variable
            return &m_variables[varIdx];
        }
    }

    // Return nullptr if there is no variable with the specified name
    return nullptr;
}

INTEGER* Program::varGetIntegerPtr(const string& varName)
{
    Variable* varTarget = Program::varFind(varName);

    // Target variable doesn't exist
    if (!varTarget)
    {
        Program::errorVarUndeclared(varName);
        return nullptr;
    }

    // Target variable must be integer
    if (varTarget->Type != DataType::Integer)
    {
        Program::error("Expected an integer variable!");
        return nullptr;
    }

    return (INTEGER*)varTarget->Pointer;
}

LOGICAL* Program::varGetLogicalPtr(const string& varName)
{
    Variable* varTarget = Program::varFind(varName);

    // Target variable doesn't exist
    if (!varTarget)
    {
        Program::errorVarUndeclared(varName);
        return nullptr;
    }

    // Target variable must be logical
    if (varTarget->Type != DataType::Logical)
    {
        Program::error("Expected a logical variable!");
        return nullptr;
    }

    return (LOGICAL*)varTarget->Pointer;
}

REAL* Program::varGetRealPtr(const string& varName)
{
    Variable* varTarget = Program::varFind(varName);

    // Target variable doesn't exist
    if (!varTarget)
    {
        Program::errorVarUndeclared(varName);
        return nullptr;
    }

    // Target variable must be real
    if (varTarget->Type != DataType::Real)
    {
        Program::error("Expected a real variable!");
        return nullptr;
    }

    return (REAL*)varTarget->Pointer;
}

enum PROGRAM_NAME Program::nameValid(const string& name)
{
    size_t namelen = name.size();

    for (size_t i = 0; i < namelen; i++)
    {
        // Only lowercase letters, uppercase letter, numbers and underscores
        // are considered as valid character in names
        if ((name.at(i) < 'a' || name.at(i) > 'z') &&
            (name.at(i) < 'A' || name.at(i) > 'Z') &&
            (name.at(i) < '0' || name.at(i) > '9') &&
            name.at(i) != '_')
        {
            return PROGRAM_NAME_INVALID_CHAR;
        }
    }

    // Name must NOT be an existing keyword
    if (name.compare("exit") ||        
        name.compare("integer") ||
        name.compare("logical") ||
        name.compare("real") ||
        name.compare("true") ||
        name.compare("false") ||
        name.compare("sqrt") ||
        name.compare("abs") ||
        name.compare("push") ||
        name.compare("pop") ||
        name.compare("print") ||
        name.compare("if") ||
        name.compare("else") ||
        name.compare("end") ||
        name.compare("break") ||
        name.compare("continue") ||
        name.compare("echo") ||
        name.compare("sub") ||
        name.compare("call") ||
        name.compare("return") ||
        name.compare("pause"))
    {
        return PROGRAM_NAME_KEYWORD;
    }

    // Name is OK and it can be safely used
    return PROGRAM_NAME_OK;
}

void* Program::valueCopy(const DataType type, const void* const source)
{
    switch (type)
    {
        case DataType::Integer:
            return memstore(*(INTEGER*)source);

        case DataType::Logical:
            return memstore(*(LOGICAL*)source);

        case DataType::Real:
            return memstore(*(REAL*)source);

        default:
            return nullptr;
    }
}

INTEGER Program::toInteger(const void* const value, const DataType type)
{
    switch (type)
    {
        case DataType::Integer:
            return *(INTEGER*)value;

        case DataType::Logical:
            return (INTEGER)*(LOGICAL*)value;

        case DataType::Real:
            return (INTEGER)*(REAL*)value;

        default:
            return 0;
    }
}

LOGICAL Program::toLogical(const void* const value, const DataType type)
{
    switch (type)
    {
        case DataType::Integer:
            return (LOGICAL)*(INTEGER*)value;

        case DataType::Logical:
            return *(LOGICAL*)value;

        case DataType::Real:
            return (LOGICAL)*(REAL*)value;

        default:
            return 0;
    }
}

REAL Program::toReal(const void* const value, const DataType type)
{
    switch (type)
    {
        case DataType::Integer:
            return (REAL)*(INTEGER*)value;

        case DataType::Logical:
            return (REAL)*(LOGICAL*)value;

        case DataType::Real:
            return *(REAL*)value;

        default:
            return 0;
    }
}

void Program::selfToInteger(void** const valueptr, DataType* const type)
{
    // Performs data type conversion and updates information
    // Swaps the pointer to the wrong data type value with the new one

    if (*type != DataType::Integer)
    {
        // Convert the value to integer
        void* valueTemp = memstore(Program::toInteger(*valueptr, *type));
        // Updates the data type information
        (*type) = DataType::Integer;
        // Frees the memory spaced used by the old value
        free(*valueptr);
        // Updates the value pointer to the converted value
        (*valueptr) = valueTemp;
    }
}

void Program::selfToLogical(void** const valueptr, DataType* const type)
{
    // Performs data type conversion and updates information
    // Swaps the pointer to the wrong data type value with the new one

    if (*type != DataType::Logical)
    {
        // Convert the value to logical
        void* valueTemp = memstore(Program::toLogical(*valueptr, *type));
        // Updates the data type information
        (*type) = DataType::Logical;
        // Frees the memory spaced used by the old value
        free(*valueptr);
        // Updates the value pointer to the converted value
        (*valueptr) = valueTemp;
    }
}

void Program::selfToReal(void** const valueptr, DataType* const type)
{
    // Performs data type conversion and updates information
    // Swaps the pointer to the wrong data type value with the new one

    if (*type != DataType::Real)
    {
        // Convert the value to real
        void* valueTemp = memstore(Program::toReal(*valueptr, *type));
        // Updates the data type information
        (*type) = DataType::Real;
        // Frees the memory spaced used by the old value
        free(*valueptr);
        // Updates the value pointer to the converted value
        (*valueptr) = valueTemp;
    }
}

void Program::toCommonType(void** const value1ptr, DataType* const type1, void** const value2ptr, DataType* const type2)
{
    // Both input values have the same data type, there's no need for a conversion
    if (*type1 == *type2)
    {
        return;
    }

    // Variable with lower data type prioroty is converted to the higher priority data type
    // real > integer > logical

    // real and integer/logical
    if (*type1 == DataType::Real)
    {
        Program::selfToReal(value2ptr, type2);
    }
    // integer/logical and real
    else if (*type2 == DataType::Real)
    {
        Program::selfToReal(value1ptr, type1);
    }
    // integer and logical
    else if (*type1 == DataType::Integer)
    {
        Program::selfToInteger(value2ptr, type2);
    }
    // logical and integer
    else if (*type2 == DataType::Integer)
    {
        Program::selfToInteger(value1ptr, type1);
    }
}

bool Program::declare(const string& strSymbol, const DataType type)
{
    string arrayName;
    size_t arraySize = 0;
    
    // Extract array information from the symbol if possible
    if (Program::symbolToArrayInfo(strSymbol, arrayName, &arraySize))
    {
        // Try to declare the specified array
        if (!Program::arrayDeclare(arrayName, type, arraySize))
        {
            // Failed to declare the array
            arrayName.dispose();
            return false;
        }

        arrayName.dispose();
    }
    else
    {
        arrayName.dispose();

        // Try to declare the specified variable
        if (!Program::varDeclare(strSymbol, type))
        {
            // Failed to declare the variable
            return false;
        }
    }

    // Success
    return true;
}

void* Program::findStorage(const string& strSymbol, DataType* const type)
{
    string arrayName;
    size_t arrayIdx = 0;
    
    // Extract array information from the symbol if possible
    if (Program::symbolToArrayInfo(strSymbol, arrayName, &arrayIdx))
    {
        // Try to find an array with the specified name
        Array* arrayPtr = Program::arrayFind(arrayName);
        arrayName.dispose();

        // Array found
        if (arrayPtr)
        {
            // Copy the data type
            (*type) = arrayPtr->Type;
            // Return pointer to the specified element
            void* elementPtr = arrayPtr->getElementPtr(arrayIdx);

            if (elementPtr)
            {
                return elementPtr;
            }
            else
            {
                Program::error("Index is out of array boundaries!");
                return nullptr;
            }
        }
    }
    // Unable to extract array information from the symbol
    else
    {
        arrayName.dispose();
        // Try to find a variable with the specified name
        Variable* varPtr = Program::varFind(strSymbol);

        if (varPtr)
        {
            // Copy the data type
            (*type) = varPtr->Type;
            // Return pointer to the value of the variable
            return varPtr->Pointer;
        }
    }

    // Unable to find a storage with the specified symbol
    Program::errorVarUndeclared(strSymbol);
    return nullptr;
}
