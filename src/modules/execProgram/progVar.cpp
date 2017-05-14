#include <modules/exec.hpp>
#include <c/stdio.h>

bool Program::varDeclare(const string& name, const DataType type)
{
    // Disabled to make recursive subroutines possible
    // Each variable must have a different name
    /*if (Program::varFind(name))
    {
        string strError;
        strError.clear();

        strError.push_back("Variable with the name \"");
        strError.push_back(name);
        strError.push_back("\" is already declared!");

        Program::error(strError);
        strError.dispose();
        return;
    }*/

    // Check variable name validity
    if (Program::nameValid(name))
    {
        m_variables.push_back(Variable());
        m_variables.back().declare(name, type, m_scope);

        // Variable declared successfully
        return true;
    }

    // Failed to declare a variable with specified name
    return false;
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

bool Program::nameValid(const string& name)
{
    size_t namelen = name.size();

    for (size_t i = 0; i < namelen; i++)
    {
        // Only lowercase letters, uppercase letter, numbers and underscores
        // are considered as valid character in variable names
        if ((name.at(i) < 'a' || name.at(i) > 'z') &&
            (name.at(i) < 'A' || name.at(i) > 'Z') &&
            (name.at(i) < '0' || name.at(i) > '9') &&
            name.at(i) != '_')
        {
            string strError;
            strError.clear();

            strError.push_back("Variable name \"");
            strError.push_back(name);
            strError.push_back("\" contains an invalid character!\nValid characters: { a-z, A-Z, 0-9, _ }");

            Program::error(strError);
            strError.dispose();

            return false;
        }
    }

    // Variable name must NOT be an existing keyword
    if (name.compare("exit") ||        
        name.compare("integer") ||
        name.compare("logical") ||
        name.compare("real") ||
        name.compare("true") ||
        name.compare("false") ||
        name.compare("push") ||
        name.compare("pop") ||
        name.compare("print") ||
        name.compare("if") ||
        name.compare("else") ||
        name.compare("end") ||
        name.compare("break") ||
        name.compare("continue") ||
        name.compare("echo"))
    {
        string strError;
        strError.clear();

        strError.push_back("Variable name \"");
        strError.push_back(name);
        strError.push_back("\" conflicts with a keyword!");

        Program::error(strError);
        strError.dispose();

        return false;
    }

    // Variable name is OK and it can be safely used
    return true;
}
