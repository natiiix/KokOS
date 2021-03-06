#include <modules/exec.hpp>
#include <c/stdio.h>

bool Program::arrayDeclare(const string& name, const DataType type, const size_t length)
{
    // Zero length array declaration attempt
    if (!length)
    {
        Program::error("Unable to declare an array with zero length!");
        return false;
    }

    // Check array name validity
    enum PROGRAM_NAME nameResult = Program::nameValid(name);
    if (nameResult == PROGRAM_NAME_OK)
    {
        // Push the new array into the array vector
        m_arrays.push_back(Array());
        m_arrays.back().declare(name, type, m_scope, length);

        // Array declared successfully
        return true;
    }
    else
    {
        string strError;
        strError.clear();

        strError.push_back("Array name \"");
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

        // Failed to define a array with specified name
        return false;
    }
}

Array* Program::arrayFind(const string& name)
{
    size_t arraysize = m_arrays.size();
    
    for (size_t i = 0; i < arraysize; i++)
    {
        // Search is performed from back to front (explained in Variable equivalent of this method)
        size_t arrayIdx = arraysize - 1 - i;

        // Find the array with the specified name
        if (m_arrays[arrayIdx].Name.compare(name))
        {
            // Return pointer to the array
            return &m_arrays[arrayIdx];
        }
    }

    // Return nullptr if there is no array with the specified name
    return nullptr;
}
