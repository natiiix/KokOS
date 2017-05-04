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
    }
    // Integer variable declaration
    else if (cmd[0].compare("integer") && cmd.size() == 2)
    {
        Program::varDeclare(cmd[1], DataType::Integer);
    }
    // Logical variable declaration
    else if (cmd[0].compare("logical") && cmd.size() == 2)
    {
        Program::varDeclare(cmd[1], DataType::Logical);
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
    // Variable value definition
    // Syntax: <Target Variable> = <Source Variable / Literal Value>
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
    // Variable value definition with operation performing
    // Syntax: <Target Variable> = <Source Variable / Literal Value> <Operator> <Source Variable / Literal Value>
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
                INTEGER valueSource1 = 0;
                INTEGER valueSource2 = 0;

                // Resolve both source values
                if (!Program::symbolToInteger(cmd[2], &valueSource1) ||
                    !Program::symbolToInteger(cmd[4], &valueSource2))
                {
                    // Unable to resolve one of the symbols
                    return;
                }

                // Addition
                if (cmd[3].compare("+"))
                {
                    varTarget->set(valueSource1 + valueSource2);
                }
                // Subtraction
                else if (cmd[3].compare("-"))
                {
                    varTarget->set(valueSource1 - valueSource2);
                }
                // Multiplication
                else if (cmd[3].compare("*"))
                {
                    varTarget->set(valueSource1 * valueSource2);
                }
                // Integer division
                else if (cmd[3].compare("/"))
                {
                    varTarget->set(valueSource1 / valueSource2);
                }
                // Remainder after integer division
                else if (cmd[3].compare("%"))
                {
                    varTarget->set(valueSource1 % valueSource2);
                }
                else
                {
                    Program::errorOperatorInvalid(cmd[3]);
                    return;
                }

                break;
            }

            case DataType::Logical:
            {
                // Check input symbols for logical values
                LOGICAL valueLogical1 = 0;
                LOGICAL valueLogical2 = 0;
                bool source1Logical = false;
                bool source2Logical = false;

                source1Logical = Program::symbolToLogical(cmd[2], &valueLogical1, false);
                source2Logical = Program::symbolToLogical(cmd[4], &valueLogical2, false);

                // Check input symbols for integer values
                INTEGER valueInteger1 = 0;
                INTEGER valueInteger2 = 0;
                bool source1Integer = false;
                bool source2Integer = false;

                source1Integer = Program::symbolToInteger(cmd[2], &valueInteger1, false);
                source2Integer = Program::symbolToInteger(cmd[4], &valueInteger2, false);

                // Both source symbols represent valid logical values
                if (source1Logical && source2Logical)
                {
                    // Boolean NXOR (equality check)
                    if (cmd[3].compare("=="))
                    {
                        varTarget->set(valueLogical1 == valueLogical2);
                    }
                    // Boolean XOR (inequality check)
                    else if (cmd[3].compare("!="))
                    {
                        varTarget->set(valueLogical1 != valueLogical2);
                    }
                    // Boolean AND
                    else if (cmd[3].compare("&&"))
                    {
                        varTarget->set(valueLogical1 && valueLogical2);
                    }
                    // Boolen OR
                    else if (cmd[3].compare("||"))
                    {
                        varTarget->set(valueLogical1 || valueLogical2);
                    }
                    else
                    {
                        Program::errorOperatorInvalid(cmd[3]);
                        return;
                    }

                    break;
                }
                // Both source symbols represent valid integer values
                else if (source1Integer && source2Integer)
                {
                    // Equality
                    if (cmd[3].compare("=="))
                    {
                        varTarget->set(valueInteger1 == valueInteger2);
                    }
                    // Inequality
                    else if (cmd[3].compare("!="))
                    {
                        varTarget->set(valueInteger1 != valueInteger2);
                    }
                    // Greater than
                    else if (cmd[3].compare(">"))
                    {
                        varTarget->set(valueInteger1 > valueInteger2);
                    }
                    // Less than
                    else if (cmd[3].compare("<"))
                    {
                        varTarget->set(valueInteger1 < valueInteger2);
                    }
                    // Greater than or equal to
                    else if (cmd[3].compare(">="))
                    {
                        varTarget->set(valueInteger1 >= valueInteger2);
                    }
                    // Less than or equal to
                    else if (cmd[3].compare("<="))
                    {
                        varTarget->set(valueInteger1 <= valueInteger2);
                    }
                    else
                    {
                        Program::errorOperatorInvalid(cmd[3]);
                        return;
                    }

                    break;
                }

                // Failed to resolve any of the symbols
                if (!source1Logical && !source2Logical && !source1Integer && !source2Integer)
                {
                    Program::errorSymbolUnresolved(cmd[2]);
                    Program::errorSymbolUnresolved(cmd[4]);
                }
                // Data types of source symbols do not match
                else if (source1Logical != source2Logical && source1Integer != source2Integer)
                {
                    Program::errorTypesIncompatible();
                }
                else
                {
                    Program::error("Unable to process source symbols!\nMake sure both source symbols represent existing values of matching data type!");
                }

                return;
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
    // Variable name must not be an existing keyword
    if (
        name.compare("exit") ||
        name.compare("integer") ||
        name.compare("real") ||
        name.compare("logical") ||
        name.compare("push") ||
        name.compare("pop"))
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
