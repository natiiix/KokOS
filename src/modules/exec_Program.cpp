#include <modules/exec.hpp>
#include <c/stdio.h>

void Program::run(const char* const codePtr)
{
    string strCode;
    strCode.set(codePtr);

    vector<string> vectLines = strCode.split('\n', false);
    strCode.dispose();

    m_program.clear();

    for (size_t i = 0; i < vectLines.size(); i++)
    {
        // Split each line into space separated words
        m_program.push_back(vectLines[i].split(' ', true));
    }

    vectLines.dispose();

    // Set program counter and scope depth
    m_counter = 0;
    m_scope = 0;
    
    // Execute commands one after another until the program finishes
    while (m_counter < m_program.size())
    {
        Program::executeCommand();
    }

    // After the program execution is done the scope level should be 0
    // This doesn't apply when the program is terminated via Program::exit()
    if (m_scope && m_counter != PROGRAM_COUNTER_EXIT)
    {
        Program::error("End of scope expected!");
    }
    
    // Dispose the whole program code
    while (m_program.size())
    {
        m_program.back().dispose();
        m_program.pop_back();
    }
    m_program.dispose();

    // Dispose all variables
    while (m_variables.size())
    {
        m_variables.back().dispose();
        m_variables.pop_back();
    }
    m_variables.dispose();

    m_scopeStack.dispose();
}

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

void Program::executeCommand(void)
{
    // Ignore empty lines and comments
    if (!m_program[m_counter].size() ||
        (m_program[m_counter][0].size() >= 2 && m_program[m_counter][0][0] == '/' && m_program[m_counter][0][1] == '/'))
    {
        m_counter++;
        return;
    }

    // I figured out that a shortcut for the current command might be useful
    vector<string>& cmd = m_program[m_counter];

    // Program exit
    if (cmd[0].compare("exit") && cmd.size() == 1)
    {
        Program::exit();
        return;
    }
    // Integer variable declaration
    else if (cmd[0].compare("integer") && cmd.size() == 2)
    {
        Program::varDeclare(cmd[1], DataType::Integer);
    }
    // Integer variable declaration with immediate value definition
    else if (cmd[0].compare("integer") && cmd.size() == 4 && cmd[2].compare("="))
    {
        INTEGER value = 0;
        if (!Program::symbolToInteger(cmd[3], &value))
        {
            return;
        }

        // Declare the variable
        Program::varDeclare(cmd[1], DataType::Integer);

        // Assign the specified value to the recently reclared variable
        m_variables.back().set(value);
    }
    // Integer variable declaration with statement evaluation
    else if (cmd[0].compare("integer") && cmd.size() == 6 && cmd[2].compare("="))
    {
        INTEGER value = false;
        if (!Program::evaluateInteger(cmd[3], cmd[4], cmd[5], &value))
        {
            return;
        }

        // Declare the variable
        Program::varDeclare(cmd[1], DataType::Integer);

        // Assign the specified value to the recently reclared variable
        m_variables.back().set(value);
    }
    // Logical variable declaration
    else if (cmd[0].compare("logical") && cmd.size() == 2)
    {
        Program::varDeclare(cmd[1], DataType::Logical);
    }
    // Logical variable declaration with immediate value definition
    else if (cmd[0].compare("logical") && cmd.size() == 4 && cmd[2].compare("="))
    {
        LOGICAL value = false;
        if (!Program::symbolToLogical(cmd[3], &value))
        {
            return;
        }

        // Declare the variable
        Program::varDeclare(cmd[1], DataType::Logical);

        // Assign the specified value to the recently reclared variable
        m_variables.back().set(value);
    }
    // Logical variable declaration with statement evaluation
    else if (cmd[0].compare("logical") && cmd.size() == 6 && cmd[2].compare("="))
    {
        LOGICAL value = false;
        if (!Program::evaluateLogical(cmd[3], cmd[4], cmd[5], &value))
        {
            return;
        }

        // Declare the variable
        Program::varDeclare(cmd[1], DataType::Logical);

        // Assign the specified value to the recently reclared variable
        m_variables.back().set(value);
    }
    // Scope push
    else if (cmd[0].compare("push") && cmd.size() == 1)
    {
        Program::scopePush();
    }
    // Scope pop
    else if (cmd[0].compare("pop") && cmd.size() == 1)
    {
        Program::scopePop();
    }
    // Variable value definition by direct assignment
    else if (cmd.size() == 3 && cmd[1].compare("="))
    {
        Variable* varTarget = Program::varFind(cmd[0]);
        
        // Target variable doesn't exist
        if (!varTarget)
        {
            Program::errorVarUndeclared(cmd[0]);
            return;
        }

        switch (varTarget->Type)
        {
            case DataType::Integer:
            {
                INTEGER value = 0;

                if (Program::symbolToInteger(cmd[2], &value))
                {
                    varTarget->set(value);
                }
                else
                {
                    return;
                }

                break;
            }

            case DataType::Logical:
            {
                LOGICAL value = 0;

                if (Program::symbolToLogical(cmd[2], &value))
                {
                    varTarget->set(value);
                }
                else
                {
                    return;
                }

                break;
            }

            default:
                break;
        }
    }
    // Variable value definition by performing an operation
    else if (cmd.size() == 5 && cmd[1].compare("="))
    {
        Variable* varTarget = Program::varFind(cmd[0]);

        // Target variable doesn't exist
        if (!varTarget)
        {
            Program::errorVarUndeclared(cmd[0]);
            return;
        }

        switch (varTarget->Type)
        {
            case DataType::Integer:
            {
                if (!Program::evaluateInteger(cmd[2], cmd[3], cmd[4], (INTEGER*)varTarget->Pointer))
                {
                    return;
                }

                break;
            }

            case DataType::Logical:
            {
                if (!Program::evaluateLogical(cmd[2], cmd[3], cmd[4], (LOGICAL*)varTarget->Pointer))
                {
                    return;
                }

                break;
            }

            default:
                break;
        }
    }
    // Variable value print
    else if (cmd[0].compare("print") && cmd.size() == 2)
    {
        Variable* varSource = Program::varFind(cmd[1]);
        
        // Source variable doesn't exist
        if (!varSource)
        {
            Program::errorVarUndeclared(cmd[1]);
            return;
        }

        switch (varSource->Type)
        {
            case DataType::Integer:
            {
                string strValue = string::toString(varSource->getInteger());
                sprint(strValue);
                newline();
                strValue.dispose();
                break;
            }

            case DataType::Logical:
            {
                string strValue = string::toString(varSource->getLogical());
                sprint(strValue);
                newline();
                strValue.dispose();
                break;
            }

            default:
                break;
        }
    }
    // if statement / while loop
    else if (cmd[0].compare("if") || cmd[0].compare("while"))
    {
        LOGICAL condition = false;

        if (cmd.size() == 2)
        {
            // Single symbol condition format
            if (!Program::symbolToLogical(cmd[1], &condition))
            {
                return;
            }
        }
        else if (cmd.size() == 4)
        {
            // Condition via comparison of 2 symbols format
            if (!Program::evaluateLogical(cmd[1], cmd[2], cmd[3], &condition))
            {
                return;
            }
        }
        else
        {
            Program::error("Unexpected condition format!");
            return;
        }

        // Condition is true, just continue the code execution
        if (condition)
        {
            Program::scopePush();
        }
        // Condition is false, skip the related code
        else
        {
            // If this is an if statement and the condition is false
            if (cmd[0].compare("if"))
            {
                // Try to find an else if / else statement, if there are none go to the end
                Program::elseLoop();
                return;
            }
            // This is not an if statement
            // else if / else statements are supposed to be ignored
            else
            {
                size_t endIndex = Program::findEnd();
                
                // end statement found
                // Jump past the end statement
                if (endIndex)
                {
                    m_counter = endIndex + 1;
                }

                return;
            }
        }
    }
    // else / else if statement
    else if (cmd[0].compare("else") &&
        (cmd.size() == 1 || ((cmd.size() == 3 || cmd.size() == 5) && cmd[1].compare("if"))))
    {
        size_t endIndex = Program::findEnd();

        if (endIndex)
        {
            m_counter = endIndex;
        }

        return;
    }
    // end statement
    else if (cmd[0].compare("end") && cmd.size() == 1)
    {
        if (m_scope)
        {
            // If this end statements belongs to a while loop
            if (m_program[m_scopeStack.back()][0].compare("while"))
            {
                // Jump back to the beginning of the loop
                m_counter = m_scopeStack.back();
                Program::scopePop();
                return;
            }

            Program::scopePop();
        }
        else
        {
            Program::error("Unexpected \"end\" statement!");
            return;
        }
    }
    // break out of the current scope
    else if (cmd[0].compare("break") && cmd.size() == 1)
    {
        if (!breakScope(1, true))
        {
            return;
        }
    }
    // break out of N scope levels
    else if (cmd[0].compare("break") && cmd.size() == 2)
    {
        INTEGER breakLevels = 0;
        
        if (Program::symbolToInteger(cmd[1], &breakLevels))
        {
            if (breakLevels > 0)
            {
                if (!breakScope(breakLevels, true))
                {
                    return;
                }
            }
            else
            {
                Program::error("Cannot break a non-positive number of scope levels!");
                return;
            }
        }
        // Symbol is not a valid integer
        else
        {
            return;
        }
    }
    // continue at the end of the current scope
    else if (cmd[0].compare("continue") && cmd.size() == 1)
    {
        breakScope(1, false);
        return;
    }
    // continue at the end of the Nth scope
    else if (cmd[0].compare("continue") && cmd.size() == 2)
    {
        INTEGER continueLevels = 0;
        
        if (Program::symbolToInteger(cmd[1], &continueLevels))
        {
            if (continueLevels > 0)
            {
                breakScope(continueLevels, false);
            }
            else
            {
                Program::error("Cannot break a non-positive number of scope levels!");
            }
        }

        return;
    }
    // Prints words to the terminal
    else if (cmd[0].compare("echo"))
    {
        // Get the number of words after the echo command
        size_t echoWordCount = cmd.size() - 1;

        // Print out each word
        for (size_t i = 0; i < echoWordCount; i++)
        {
            // Make spaces between words
            if (i)
            {
                print(" ");
            }

            // Print out the word itself
            sprint (cmd[1 + i]);
        }

        // Break the line
        newline();
    }
    // Increments an integer variable
    else if (cmd.size() == 2 && cmd[1].compare("++"))
    {
        // Find the variable
        INTEGER* varPtr = Program::varGetIntegerPtr(cmd[0]);

        // Increment the value of the variable
        (*varPtr)++;
    }
    // Decrements an integer variable
    else if (cmd.size() == 2 && cmd[1].compare("--"))
    {
        // Find the variable
        INTEGER* varPtr = Program::varGetIntegerPtr(cmd[0]);

        // Decrement the value of the variable
        (*varPtr)--;
    }
    // Unrecognized command
    else
    {
        string strErrorMsg;        
        strErrorMsg.clear();
        
        strErrorMsg.push_back("Unrecognized command: \"");

        for (size_t i = 0; i < cmd.size(); i++)
        {
            if (i)
            {
                strErrorMsg.push_back(' ');
            }

            strErrorMsg.push_back(cmd[i]);
        }

        strErrorMsg.push_back('\"');

        Program::error(strErrorMsg);
        strErrorMsg.dispose();
        return;
    }
    
    // If the program is supposed to exit it should never reach the program counter incrementation
    if (m_counter == PROGRAM_COUNTER_EXIT)
    {
        debug_print("exec_Program.cpp | Program::executeCommand() | Missing return statement after program exit detected!");
        return;
    }

    // Progress to next command
    m_counter++;
}

void Program::varDeclare(const string& name, const DataType type)
{
    // Each variable must have a different name
    if (Program::varFind(name))
    {
        string strError;
        strError.clear();

        strError.push_back("Variable with the name \"");
        strError.push_back(name);
        strError.push_back("\" is already declared!");

        Program::error(strError);
        strError.dispose();
        return;
    }

    // Keywords cannot be used as variable names
    if (Program::varNameIsKeyword(name))
    {
        string strError;
        strError.clear();

        strError.push_back("Variable name \"");
        strError.push_back(name);
        strError.push_back("\" conflicts with a keyword!");

        Program::error(strError);
        strError.dispose();
        return;
    }

    // Check for invalid characters
    if (Program::varNameInvalidChar(name))
    {
        string strError;
        strError.clear();

        strError.push_back("Variable name \"");
        strError.push_back(name);
        strError.push_back("\" contains an invalid character!\nValid characters: { a-z, A-Z, 0-9, _ }");

        Program::error(strError);
        strError.dispose();
        return;
    }

    m_variables.push_back(Variable());
    m_variables.back().declare(name, type, m_scope);
}

Variable* Program::varFind(const string& name)
{
    size_t varsize = m_variables.size();
    
    for (size_t i = 0; i < varsize; i++)
    {
        // Find the variable with the specified name
        if (m_variables[i].Name.compare(name))
        {
            // Return pointer to the variable
            return &m_variables[i];
        }
    }

    // Return nullptr if there is no variable with the specified name
    return nullptr;
}

bool Program::varNameIsKeyword(const string& name)
{
    // Variable name must NOT be an existing keyword
    if (name.compare("exit") ||        
        name.compare("integer") ||
        name.compare("logical") ||
        name.compare("push") ||
        name.compare("pop") ||
        name.compare("print") ||
        name.compare("if") ||
        name.compare("else") ||
        name.compare("end") ||
        name.compare("break"))
    {
        return true;
    }

    return false;
}

bool Program::varNameInvalidChar(const string& name)
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
            return true;
        }
    }

    return false;
}

void Program::exit(void)
{
    m_counter = PROGRAM_COUNTER_EXIT;
}

void Program::error(const char* const str)
{
    print("Error on line ");
    printint(m_counter);
    newline();
    print(str);
    newline();

    Program::exit();
}

void Program::error(const string& str)
{
    Program::error(str.c_str());
}

void Program::errorVarUndeclared(const string& strVarName)
{
    string strErrorMsg;
    strErrorMsg.clear();
    
    strErrorMsg.push_back("Variable \"");
    strErrorMsg.push_back(strVarName);
    strErrorMsg.push_back("\" has not been declared in current scope!");

    Program::error(strErrorMsg);
    strErrorMsg.dispose();
}

void Program::errorSymbolUnresolved(const string& strSymbol)
{
    string strError;
    strError.clear();

    strError.push_back("Unable to resolve symbol \"");
    strError.push_back(strSymbol);
    strError.push_back("\"!");

    Program::error(strError);
    strError.dispose();
}

void Program::errorTypesIncompatible(void)
{
    Program::error("Variables do not have matching data types!");
}

void Program::errorOperatorInvalid(const string& strOperator)
{
    string strError;
    strError.clear();

    strError.push_back("Symbol \"");
    strError.push_back(strOperator);
    strError.push_back("\" is not a valid operator!");

    Program::error(strError);
    strError.dispose();
}

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
            if (m_program[elseIndex][0].compare("else") &&
                (m_program[elseIndex].size() == 3 || m_program[elseIndex].size() == 5) &&
                m_program[elseIndex][1].compare("if"))
            {
                LOGICAL elseCondition = false;
                    
                // else if statement using a single symbol condition
                if (m_program[elseIndex].size() == 3)
                {
                    if (!Program::symbolToLogical(m_program[elseIndex][2], &elseCondition))
                    {
                        break;
                    }
                }
                // else if statement that uses a comparison as a condition
                else if (m_program[elseIndex].size() == 5)
                {
                    if (!Program::evaluateLogical(m_program[elseIndex][2], m_program[elseIndex][3], m_program[elseIndex][4], &elseCondition))
                    {
                        break;
                    }
                }

                // Condition of this else if is satisfied, continue the code execution
                if (elseCondition)
                {
                    m_counter = elseIndex + 1;
                    Program::scopePush();
                    break;
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
            break;
        }
        else
        {
            break;
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
        Program::error("Expected an integer!");
        return nullptr;
    }

    return (INTEGER*)varTarget->Pointer;
}
