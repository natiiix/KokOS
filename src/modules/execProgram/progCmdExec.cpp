#include <modules/exec.hpp>
#include <c/stdio.h>

void Program::executeCommand(void)
{
    // Ignore empty lines and comments
    if (!m_program[m_counter].size() ||
        (m_program[m_counter][0].size() >= 2 && m_program[m_counter][0][0] == '/' && m_program[m_counter][0][1] == '/'))
    {
        m_counter++;
        return;
    }

    // Clear the input buffer
    // If Escape was pressed terminate the program
    keyevent ke;
    do
    {
        // Read key from input buffer
        ke = readKeyEvent();
        
        // Check if Escape key was pressed
        if (ke.scancode == KEY_ESCAPE &&
            ke.state &&
            !ke.modifiers)
        {
            // Terminate the program
            print("Program execution was manually terminated by user!\n");
            m_counter = PROGRAM_COUNTER_EXIT;
            return;
        }
    }
    // Continue until the input buffer is empty
    while (ke.scancode);

    // I figured out that a shortcut for the current command might be useful
    vector<string>& cmd = m_program[m_counter];

    // Program exit
    if (cmd[0].compare("exit") && cmd.size() == 1)
    {
        m_counter = PROGRAM_COUNTER_EXIT;
        return;
    }
    // Integer variable declaration
    else if (cmd[0].compare("integer") && cmd.size() == 2)
    {
        if (!Program::varDeclare(cmd[1], DataType::Integer))
        {
            return;
        }
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
        if (!Program::varDeclare(cmd[1], DataType::Integer))
        {
            return;
        }

        // Assign the specified value to the recently reclared variable
        m_variables.back().set(value);
    }
    // Integer variable declaration with value definition by performing a type conversion
    else if (cmd[0].compare("integer") && cmd.size() == 5 && cmd[2].compare("="))
    {
        // Declare the variable
        if (!Program::varDeclare(cmd[1], DataType::Integer))
        {
            return;
        }

        // Data type conversion
        if (cmd[3].compare("convert"))
        {
            // Assign the converted value to the recently reclared variable
            if (!Program::convertToInteger(cmd.at(4), &m_variables.back()))
            {
                // Unable to perform the type conversion
                return;
            }
        }
        // Unable to recognize the operation
        else
        {
            Program::errorOperatorInvalid(cmd.at(2));
            return;
        }
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
        if (!Program::varDeclare(cmd[1], DataType::Integer))
        {
            return;
        }

        // Assign the specified value to the recently reclared variable
        m_variables.back().set(value);
    }
    // Logical variable declaration
    else if (cmd[0].compare("logical") && cmd.size() == 2)
    {
        if (!Program::varDeclare(cmd[1], DataType::Logical))
        {
            return;
        }
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
        if (!Program::varDeclare(cmd[1], DataType::Logical))
        {
            return;
        }

        // Assign the specified value to the recently reclared variable
        m_variables.back().set(value);
    }
    // Logical variable declaration with value definition by performing a type conversion
    else if (cmd[0].compare("logical") && cmd.size() == 5 && cmd[2].compare("="))
    {
        // Declare the variable
        if (!Program::varDeclare(cmd[1], DataType::Logical))
        {
            return;
        }

        // Data type conversion
        if (cmd[3].compare("convert"))
        {
            // Assign the converted value to the recently reclared variable
            if (!Program::convertToLogical(cmd.at(4), &m_variables.back()))
            {
                // Unable to perform the type conversion
                return;
            }
        }
        // Unable to recognize the operation
        else
        {
            Program::errorOperatorInvalid(cmd.at(2));
            return;
        }
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
        if (!Program::varDeclare(cmd[1], DataType::Logical))
        {
            return;
        }

        // Assign the specified value to the recently reclared variable
        m_variables.back().set(value);
    }
    // Real variable declaration
    else if (cmd[0].compare("real") && cmd.size() == 2)
    {
        if (!Program::varDeclare(cmd[1], DataType::Real))
        {
            return;
        }
    }
    // Real variable declaration with immediate value definition
    else if (cmd[0].compare("real") && cmd.size() == 4 && cmd[2].compare("="))
    {
        REAL value = 0;
        if (!Program::symbolToReal(cmd[3], &value))
        {
            return;
        }

        // Declare the variable
        if (!Program::varDeclare(cmd[1], DataType::Real))
        {
            return;
        }

        // Assign the specified value to the recently reclared variable
        m_variables.back().set(value);
    }
    // Real variable declaration with value definition by performing an operation
    else if (cmd[0].compare("real") && cmd.size() == 5 && cmd[2].compare("="))
    {
        // Declare the variable
        if (!Program::varDeclare(cmd[1], DataType::Real))
        {
            return;
        }

        // Square root
        if (cmd[3].compare("sqrt"))
        {
            // Assign the square root value to the recently reclared variable
            if (!Program::realSqrt(cmd.at(4), &m_variables.back()))
            {
                // Unable to perform sqaure root
                return;
            }
        }
        // Data type conversion
        else if (cmd[3].compare("convert"))
        {
            // Assign the converted value to the recently reclared variable
            if (!Program::convertToReal(cmd.at(4), &m_variables.back()))
            {
                // Unable to perform the type conversion
                return;
            }
        }
        // Unable to recognize the operation
        else
        {
            Program::errorOperatorInvalid(cmd.at(2));
            return;
        }
    }
    // Real variable declaration with statement evaluation
    else if (cmd[0].compare("real") && cmd.size() == 6 && cmd[2].compare("="))
    {
        REAL value = false;
        if (!Program::evaluateReal(cmd[3], cmd[4], cmd[5], &value))
        {
            return;
        }

        // Declare the variable
        if (!Program::varDeclare(cmd[1], DataType::Real))
        {
            return;
        }

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

            case DataType::Real:
            {
                string strValue = string::toString(varSource->getReal());
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
            // Create a shortcut for the command string from the last scope push event
            string& strLastPushCmd = m_program[m_scopeStack.back()][0];

            // End statement belongs to a while loop
            if (strLastPushCmd.compare("while"))
            {
                // Jump back to the beginning of the loop
                m_counter = m_scopeStack.back();
                Program::scopePop();
                return;
            }
            // End statement belongs to a subroutine
            else if (strLastPushCmd.compare("call"))
            {
                // Jump back to the call command
                m_counter = m_scopeStack.back();
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
    // Read variable value from terminal
    else if (cmd[0].compare("read") && cmd.size() == 2)
    {
        // Find the variable in which the value is supposed to be stored
        Variable* varTarget = Program::varFind(cmd[1]);

        // Check if the target variable exists
        if (!varTarget)
        {
            Program::errorVarUndeclared(cmd[1]);
            return;
        }

        // Read input from the terminal
        string strInput = readline();

        // Integer target variable
        if (varTarget->Type == DataType::Integer)
        {
            INTEGER inputValue = 0;

            if (strInput.parseInt32(&inputValue))
            {
                varTarget->set(inputValue);
            }
            // Integer parsing failed
            else
            {
                string strError;
                strError.clear();

                strError.push_back("Symbol \"");
                strError.push_back(strInput);
                strInput.dispose();
                strError.push_back("\" doesn't represent a valid integer value!");

                Program::error(strError);
                strError.dispose();

                return;
            }
        }
        // Logical target variable
        else if (varTarget->Type == DataType::Logical)
        {
            LOGICAL inputValue = 0;

            if (strInput.parseBool(&inputValue))
            {
                varTarget->set(inputValue);
            }
            // Logical value parsing failed
            else
            {
                string strError;
                strError.clear();

                strError.push_back("Symbol \"");
                strError.push_back(strInput);
                strInput.dispose();
                strError.push_back("\" doesn't represent a valid logical value!");

                Program::error(strError);
                strError.dispose();

                return;
            }
        }
        // Real target variable
        else if (varTarget->Type == DataType::Real)
        {
            REAL inputValue = 0;

            if (strInput.parseDouble(&inputValue))
            {
                varTarget->set(inputValue);
            }
            // Real value parsing failed
            else
            {
                string strError;
                strError.clear();

                strError.push_back("Symbol \"");
                strError.push_back(strInput);
                strInput.dispose();
                strError.push_back("\" doesn't represent a valid real value!");

                Program::error(strError);
                strError.dispose();

                return;
            }
        }

        strInput.dispose();
    }
    // Subroutine code
    else if (cmd[0].compare("sub") && cmd.size() == 2)
    {
        size_t endIdx = Program::findEnd();
        
        // End of the subroutine found
        if (endIdx)
        {
            // Jump past the end of the subroutine
            m_counter = endIdx + 1;
        }

        return;
    }
    // Subroutine call
    else if (cmd[0].compare("call") && cmd.size() == 2)
    {
        // Find the subroutine by name
        size_t subCounter = Program::subFind(cmd[1]);

        // Undefined subroutine
        if (subCounter == PROGRAM_COUNTER_EXIT)
        {
            Program::errorSubUndefined(cmd[1]);
            return;
        }

        // Push the scope at current line
        Program::scopePush();
        // Jump into the subroutine
        m_counter = subCounter + 1;
        return;
    }
    // Increments an integer variable
    else if (cmd.size() == 2 && cmd[1].compare("++"))
    {
        // Find the variable
        INTEGER* varPtr = Program::varGetIntegerPtr(cmd[0]);

        if (varPtr)
        {
            // Increment the value of the variable
            (*varPtr)++;
        }
        else
        {
            return;
        }
    }
    // Decrements an integer variable
    else if (cmd.size() == 2 && cmd[1].compare("--"))
    {
        // Find the variable
        INTEGER* varPtr = Program::varGetIntegerPtr(cmd[0]);

        if (varPtr)
        {
            // Decrement the value of the variable
            (*varPtr)--;
        }
        else
        {
            return;
        }
    }
    // Stores the square root of a real value into a real variable
    else if (cmd.size() == 4 && cmd[1].compare("=") && cmd[2].compare("sqrt"))
    {
        // Find target variable
        Variable* varTarget = Program::varFind(cmd[0]);
        
        // Target variable doesn't exist
        if (!varTarget)
        {
            Program::errorVarUndeclared(cmd[0]);
            return;
        }

        // Square root can only be performend on real data type
        if (varTarget->Type != DataType::Real)
        {
            Program::errorTypesIncompatible();
            return;
        }

        // Perform the square root
        if (!Program::realSqrt(cmd.at(3), varTarget))
        {
            // Square root failed
            return;
        }
    }
    // Variable value assignment
    else if ((cmd.size() >= 3 && cmd.size() <= 5) && cmd[1].contains('='))
    {
        // Find target variable
        Variable* varTarget = Program::varFind(cmd[0]);
        
        // Target variable doesn't exist
        if (!varTarget)
        {
            Program::errorVarUndeclared(cmd[0]);
            return;
        }

        switch (varTarget->Type)
        {
            // Target variable is integer
            case DataType::Integer:
            {
                // Assignment with data type conversion
                if (cmd.size() == 4 && cmd.at(2).compare("convert"))
                {
                    if (!Program::convertToInteger(cmd.at(3), varTarget))
                    {
                        return;
                    }

                    break;
                }

                INTEGER oldValue = varTarget->getInteger();
                INTEGER operandValue = 0;

                if ((cmd.size() == 3 && Program::symbolToInteger(cmd[2], &operandValue)) ||
                    (cmd.size() == 5 && Program::evaluateInteger(cmd[2], cmd[3], cmd[4], &operandValue)))
                {
                    // Value assignment
                    if (cmd[1].compare("="))
                    {
                        varTarget->set(operandValue);
                    }
                    // Add operand value to itself
                    else if (cmd[1].compare("+="))
                    {
                        varTarget->set(oldValue + operandValue);
                    }
                    // Subtract operand value from itself
                    else if (cmd[1].compare("-="))
                    {
                        varTarget->set(oldValue - operandValue);
                    }
                    // Multiply itself by operand value
                    else if (cmd[1].compare("*="))
                    {
                        varTarget->set(oldValue * operandValue);
                    }
                    // Perform integer division by operand value on itself
                    else if (cmd[1].compare("/="))
                    {
                        if (operandValue == 0)
                        {
                            Program::errorDivisionByZero();
                            return;
                        }

                        varTarget->set(oldValue / operandValue);
                    }
                    // Set itself to the remained after integer division by the operand value
                    else if (cmd[1].compare("%="))
                    {
                        if (operandValue == 0)
                        {
                            Program::errorDivisionByZero();
                            return;
                        }

                        varTarget->set(oldValue % operandValue);
                    }
                    // Bit shift itself left
                    else if (cmd[1].compare("<<="))
                    {
                        varTarget->set(oldValue << operandValue);
                    }
                    // Bit shift itself right
                    else if (cmd[1].compare(">>="))
                    {
                        varTarget->set(oldValue >> operandValue);
                    }
                    // Self-cast bitwise AND
                    else if (cmd[1].compare("&="))
                    {
                        varTarget->set(oldValue & operandValue);
                    }
                    // Self-cast bitwise OR
                    else if (cmd[1].compare("|="))
                    {
                        varTarget->set(oldValue | operandValue);
                    }
                    // Self-cast bitwise XOR
                    else if (cmd[1].compare("^="))
                    {
                        varTarget->set(oldValue ^ operandValue);
                    }
                    // Not a valid integer operator
                    else
                    {
                        Program::errorOperatorInvalid(cmd[1]);
                        return;
                    }
                }
                // Invalid right side operand symbol
                else
                {
                    return;
                }

                break;
            }

            // Target variable is logical
            case DataType::Logical:
            {
                // Assignment with data type conversion
                if (cmd.size() == 4 && cmd.at(2).compare("convert"))
                {
                    if (!Program::convertToLogical(cmd.at(3), varTarget))
                    {
                        return;
                    }
                    
                    break;
                }

                LOGICAL oldValue = varTarget->getLogical();
                LOGICAL operandValue = 0;

                if ((cmd.size() == 3 && Program::symbolToLogical(cmd[2], &operandValue)) ||
                    (cmd.size() == 5 && Program::evaluateLogical(cmd[2], cmd[3], cmd[4], &operandValue)))
                {
                    // Value assignment
                    if (cmd[1].compare("="))
                    {
                        varTarget->set(operandValue);
                    }
                    // Self-cast AND
                    else if (cmd[1].compare("&&="))
                    {
                        varTarget->set(oldValue && operandValue);
                    }
                    // Self-cast OR
                    else if (cmd[1].compare("||="))
                    {
                        varTarget->set(oldValue || operandValue);
                    }
                    // Not a valid logical operator
                    else
                    {
                        Program::errorOperatorInvalid(cmd[1]);
                        return;
                    }
                }
                // Invalid right side operand symbol
                else
                {
                    return;
                }

                break;
            }

            // Target variable is real
            case DataType::Real:
            {
                // Assignment with data type conversion
                if (cmd.size() == 4 && cmd.at(2).compare("convert"))
                {
                    if (!Program::convertToReal(cmd.at(3), varTarget))
                    {
                        return;
                    }
                    
                    break;
                }

                REAL oldValue = varTarget->getReal();
                REAL operandValue = 0;

                if ((cmd.size() == 3 && Program::symbolToReal(cmd[2], &operandValue)) ||
                    (cmd.size() == 5 && Program::evaluateReal(cmd[2], cmd[3], cmd[4], &operandValue)))
                {
                    // Value assignment
                    if (cmd[1].compare("="))
                    {
                        varTarget->set(operandValue);
                    }
                    // Add operand value to itself
                    else if (cmd[1].compare("+="))
                    {
                        varTarget->set(oldValue + operandValue);
                    }
                    // Subtract operand value from itself
                    else if (cmd[1].compare("-="))
                    {
                        varTarget->set(oldValue - operandValue);
                    }
                    // Multiply itself by operand value
                    else if (cmd[1].compare("*="))
                    {
                        varTarget->set(oldValue * operandValue);
                    }
                    // Perform real number division by operand value on itself
                    else if (cmd[1].compare("/="))
                    {
                        if (operandValue == 0)
                        {
                            Program::errorDivisionByZero();
                            return;
                        }

                        varTarget->set(oldValue / operandValue);
                    }
                    // Not a valid real operator
                    else
                    {
                        Program::errorOperatorInvalid(cmd[1]);
                        return;
                    }
                }
                // Invalid right side operand symbol
                else
                {
                    return;
                }

                break;
            }

            default:
                Program::error("Invalid variable data type!");
                return;
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
