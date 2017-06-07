#include <modules/exec.hpp>
#include <c/stdio.h>

bool Program::subDefine(const string& name, const size_t counter)
{
    // Each subroutine must have unique name
    // Exit counter index means that the subroutine name is unused
    if (Program::subFind(name) != PROGRAM_COUNTER_EXIT)
    {
        Program::errorScan(counter, "Subroutine with this name already exists!");
        return false;
    }

    // Check subroutine name validity
    enum PROGRAM_NAME nameResult = Program::nameValid(name);
    if (nameResult == PROGRAM_NAME_OK)
    {
        // Store the found subroutine in the subroutine vector
        m_subroutines.push_back(Subroutine());
        m_subroutines.back().define(name, counter);

        // Subroutine was successfully defined
        return true;
    }
    else
    {
        string strError;
        strError.clear();

        strError.push_back("Subroutine name \"");
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

        Program::errorScan(counter, strError.c_str());
        strError.dispose();

        // Failed to define a subroutine with specified name
        return false;
    }
}

size_t Program::subFind(const string& name)
{
    size_t subsize = m_subroutines.size();
    
    for (size_t i = 0; i < subsize; i++)
    {
        // Find the subroutine with the specified name
        if (m_subroutines[i].Name.compare(name))
        {
            // Return the counter index of the subroutine
            return m_subroutines[i].Counter;
        }
    }

    // Return the exit counter value if the specified subroutine is undefined
    return PROGRAM_COUNTER_EXIT;
}
