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
        name.compare("convert") ||
        name.compare("push") ||
        name.compare("pop") ||
        name.compare("print") ||
        name.compare("if") ||
        name.compare("else") ||
        name.compare("end") ||
        name.compare("break") ||
        name.compare("continue") ||
        name.compare("echo") ||
        name.compare("sub"))
    {
        return PROGRAM_NAME_KEYWORD;
    }

    // Name is OK and it can be safely used
    return PROGRAM_NAME_OK;
}
