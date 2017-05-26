#include <modules/exec.hpp>
#include <c/stdio.h>

void Program::scopePush(void)
{
    // Push current program counter onto the scope stack vector
    m_scopeStack.push_back(m_counter);
    m_scope++;
}

void Program::scopePop(void)
{
    // Dispose all variables declared inside of this scope level
    size_t varsize = m_variables.size();
    for (size_t i = 0; i < varsize; i++)
    {
        // If a variable has a lower scope level than the one that's being popped
        // there shouldn't be any more variables declared in this scope before it
        if (m_variables[varsize - 1 - i].Scope < m_scope)
        {
            break;
        }

        // Dispose the variable and pop it from the vector
        m_variables.back().dispose();
        m_variables.pop_back();
    }

    // Same as above except for arrays
    size_t arraysize = m_arrays.size();
    for (size_t i = 0; i < arraysize; i++)
    {
        if (m_arrays[arraysize - 1 - i].Scope < m_scope)
        {
            break;
        }

        m_arrays.back().dispose();
        m_arrays.pop_back();
    }

    // Scope pop cannot be performed if the scope is already at 0 level
    if (m_scope)
    {
        m_scope--;
        m_scopeStack.pop_back();
    }
    else
    {
        Program::error("Unexpected end of scope!");
    }
}

size_t Program::findEnd(void)
{
    size_t innerScope = 0;

    // Find the end statement of the current scope
    // Ignore all the inner scopes by counting inner scope levels
    for (size_t i = m_counter + 1; i < m_program.size(); i++)
    {
        // if and while statements increase the scope depth level
        if (m_program[i][0].compare("if") ||
            m_program[i][0].compare("while"))
        {
            // Increment the temporary scope level
            innerScope++;
        }
        // end statements decrease the scope depth level if there is any
        else if (m_program[i][0].compare("end"))
        {
            if (innerScope)
            {
                // Decrement the temporary scope level
                innerScope--;
            }
            else
            {
                // Return the number of the line with the corresponding end statement
                return i;
            }
        }
    }

    // Couldn't find the corresponsing end statement
    Program::error("Expected an \"end\" statement!");
    return 0;
}

size_t Program::findElse(void)
{
    // Find the nearest else / else if statement related to this if statement
    // If an end statement is reached before any else / else if is found
    // the index of that end statement is returned instead

    size_t innerScope = 0;

    for (size_t i = m_counter + 1; i < m_program.size(); i++)
    {
        // if and while statements increase the scope depth level
        if (m_program[i][0].compare("if") ||
            m_program[i][0].compare("while"))
        {
            // Increment the temporary scope level
            innerScope++;
        }
        // If the program is inside an inner scope
        else if (innerScope)
        {
            // If this end statement belongs to an inner if statement
            if (m_program[i][0].compare("end"))
            {
                // Decrement the temporary scope level
                innerScope--;
            }
        }
        // If the program is on the level of this if statement
        else
        {
            // Found an else statement that belongs to this if statement
            if ((m_program[i][0].compare("else") &&
                (m_program[i].size() == 1 ||
                    ((m_program[i].size() == 3 || m_program[i].size() == 5)
                        && m_program[i][1].compare("if")))) ||
                // End of this if statement reached
                m_program[i][0].compare("end"))
            {
                return i;
            }
        }
    }

    Program::error("Unexpected end of program! Expected an end statement!");
    return 0;
}

void Program::elseLoop(void)
{
    // Go through the code until an else if with satisfied condition is found or else / end is reached
    while (true)
    {
        size_t elseIndex = Program::findElse();

        if (elseIndex)
        {
            // else if statement
            if (m_program[elseIndex][0].compare("else") &&
                m_program[elseIndex].size() >= 3 &&
                m_program[elseIndex][1].compare("if"))
            {
                // Resolve the condition symbol
                LOGICAL* conditionPtr = Program::symbolMultiResolveLogical(m_program[elseIndex], 2);

                // Unable to resolve the symbol
                if (!conditionPtr)
                {
                    return;
                }

                LOGICAL condition = *conditionPtr;
                delete conditionPtr;

                // Condition of this else if is satisfied, continue the code execution
                if (condition)
                {
                    m_counter = elseIndex + 1;
                    Program::scopePush();
                    return;
                }
                // Condition is not satisfied, look for the next else if / else / end statement
                else
                {
                    m_counter = elseIndex;
                    continue;
                }
            }

            // Simple else only statement or end statement reached
            // The only thing that can be done is to continue
            m_counter = elseIndex + 1;
            return;
        }
        else
        {
            return;
        }
    }
}

bool Program::breakScope(const size_t levelsToBreak, const bool breakLast)
{
    // Cannot break more scope levels than how many there currently are
    if (m_scope < levelsToBreak)
    {
        Program::error("Number of scopes exceeds scope depth!");
        return false;
    }

    // Break out of scopes one by one
    for (size_t i = 0; i < levelsToBreak; i++)
    {
        // Find the end of this scope
        size_t endIndex = findEnd();

        // End of scope found
        if (endIndex)
        {
            m_counter = endIndex;

            // continue command doesn't break the last scope
            // The last end statement is processed in the next iteration
            if (i < levelsToBreak - 1 || breakLast)
            {
                Program::scopePop();
            }
        }
        // Unable to find the end of scope
        else
        {
            return false;
        }
    }

    return true;
}
