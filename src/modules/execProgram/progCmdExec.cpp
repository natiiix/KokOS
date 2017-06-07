#include <modules/exec.hpp>
#include <modules/shell.hpp>
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
    // Integer variable/array declaration
    else if (cmd[0].compare("integer") && cmd.size() == 2)
    {
        if (!Program::declare(cmd.at(1), DataType::Integer))
        {
            return;
        }
    }
    // Integer variable declaration with immediate value definition
    else if (cmd[0].compare("integer") && cmd.size() > 3 && cmd[2].compare("="))
    {
        // Resolve the input value from symbols
        INTEGER* value = Program::symbolMultiResolveInteger(cmd, 3);

        // Symbol resolving failed
        if (!value)
        {
            return;
        }

        // Declare the variable
        if (!Program::varDeclare(cmd[1], DataType::Integer))
        {
            delete value;
            return;
        }

        // Set the variable value to the input value
        m_variables.back().set(*value);
        delete value;
    }
    // Logical variable/array declaration
    else if (cmd[0].compare("logical") && cmd.size() == 2)
    {
        if (!Program::declare(cmd.at(1), DataType::Logical))
        {
            return;
        }
    }
    // Logical variable declaration with immediate value definition
    else if (cmd[0].compare("logical") && cmd.size() > 3 && cmd[2].compare("="))
    {
        // Resolve the input value from symbols
        LOGICAL* value = Program::symbolMultiResolveLogical(cmd, 3);

        // Symbol resolving failed
        if (!value)
        {
            return;
        }

        // Declare the variable
        if (!Program::varDeclare(cmd[1], DataType::Logical))
        {
            delete value;
            return;
        }

        // Set the variable value to the input value
        m_variables.back().set(*value);
        delete value;
    }
    // Real variable/array declaration
    else if (cmd[0].compare("real") && cmd.size() == 2)
    {
        if (!Program::declare(cmd.at(1), DataType::Real))
        {
            return;
        }
    }
    // Real variable declaration with immediate value definition
    else if (cmd[0].compare("real") && cmd.size() > 3 && cmd[2].compare("="))
    {
        // Resolve the input value from symbols
        REAL* value = Program::symbolMultiResolveReal(cmd, 3);

        // Symbol resolving failed
        if (!value)
        {
            return;
        }

        // Declare the variable
        if (!Program::varDeclare(cmd[1], DataType::Real))
        {
            delete value;
            return;
        }

        // Set the variable value to the input value
        m_variables.back().set(*value);
        delete value;
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
    // Print the value of the input symbol
    else if (cmd[0].compare("print") && cmd.size() >= 2)
    {
        // Resolve the input symbol
        DataType type;
        void* valuePtr = Program::symbolMultiResolve(cmd, 1, &type);
        
        // Invalid input symbol
        if (!valuePtr)
        {
            return;
        }

        switch (type)
        {
            case DataType::Integer:
            {
                string strValue = string::toString(*(INTEGER*)valuePtr);
                sprint(strValue);
                newline();
                strValue.dispose();
                break;
            }

            case DataType::Logical:
            {
                string strValue = string::toString(*(LOGICAL*)valuePtr);
                sprint(strValue);
                newline();
                strValue.dispose();
                break;
            }

            case DataType::Real:
            {
                string strValue = string::toString(*(REAL*)valuePtr);
                sprint(strValue);
                newline();
                strValue.dispose();
                break;
            }

            default:
                break;
        }

        free(valuePtr);
    }
    // if statement / while loop
    else if (cmd[0].compare("if") || cmd[0].compare("while"))
    {
        // Resolve the condition
        LOGICAL* conditionPtr = Program::symbolMultiResolveLogical(cmd, 1);

        // Unable to resolve the condition
        if (!conditionPtr)
        {
            return;
        }

        // Extract the logical value from the pointer
        LOGICAL condition = *conditionPtr;
        delete conditionPtr;

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
    else if (cmd[0].compare("break") && cmd.size() >= 2)
    {
        // Resolve the number of levels to break
        INTEGER* levelsPtr = Program::symbolMultiResolveInteger(cmd, 1);
        
        // Failed to resolve the number
        if (!levelsPtr)
        {
            return;
        }

        // Extract the integer value from the pointer
        INTEGER levels = *levelsPtr;
        delete levelsPtr;

        // Check if the number of scopes to break is positive
        if (levels > 0)
        {
            // Try to break the specified number of scope levels
            if (!breakScope(levels, true))
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
    // continue at the end of the current scope
    else if (cmd[0].compare("continue") && cmd.size() == 1)
    {
        breakScope(1, false);
        return;
    }
    // continue at the end of the Nth scope
    else if (cmd[0].compare("continue") && cmd.size() == 2)
    {
        // Resolve the number of levels to continue after
        INTEGER* levelsPtr = Program::symbolMultiResolveInteger(cmd, 1);
        
        // Failed to resolve the number
        if (!levelsPtr)
        {
            return;
        }

        // Extract the integer value from the pointer
        INTEGER levels = *levelsPtr;
        delete levelsPtr;

        if (levels > 0)
        {
            breakScope(levels, false);
        }
        else
        {
            Program::error("Cannot continue after a non-positive number of scope levels!");
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
            sprint(cmd[1 + i]);
        }

        // Break the line
        newline();
    }
    // Read variable value from terminal
    else if (cmd[0].compare("read") && cmd.size() == 2)
    {
        // Find the variable in which the value is supposed to be stored
        DataType type;
        void* targetPtr = Program::findStorage(cmd.at(1), &type);

        // Check if the target variable exists
        if (!targetPtr)
        {
            return;
        }

        // Read input from the terminal
        string strInput = readline();

        // Integer target variable
        if (type == DataType::Integer)
        {
            INTEGER inputValue = 0;

            if (strInput.parseInt32(&inputValue))
            {
                (*(INTEGER*)targetPtr) = inputValue;
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
        else if (type == DataType::Logical)
        {
            LOGICAL inputValue = 0;

            if (strInput.parseBool(&inputValue))
            {
                (*(LOGICAL*)targetPtr) = inputValue;
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
        else if (type == DataType::Real)
        {
            REAL inputValue = 0;

            if (strInput.parseDouble(&inputValue))
            {
                (*(REAL*)targetPtr) = inputValue;
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
    // Premature end of subroutine
    else if (cmd[0].compare("return") && cmd.size() == 1)
    {
        while (m_scope)
        {
            // The subroutine is the currently deepest scope level
            if (m_program[m_scopeStack.back()][0].compare("call"))
            {
                // Jump back to the call command
                m_counter = m_scopeStack.back() + 1;
                // Pop the subroutine scope
                Program::scopePop();
                return;
            }
            // Pop all the inner scopes (inside of the subroutine) until the subroutine call is reached
            else
            {
                Program::scopePop();
            }
        }
        
        // Ran out of scope levels before finding the subroutine call
        Program::error("Unexpected \"return\" statement!");
        return;
    }
    // "Press ENTER key to continue..." from the program code
    else if (cmd[0].compare("pause") && cmd.size() == 1)
    {
        // Display memory usage (used during Exec development to make it easier to find memory leaks)
        debug_memusage();
        // Required the user to press the Enter key in order to continue the program execution
        pause();
    }
    // Execution of shell command programatically
    else if (cmd[0].compare("shell") && cmd.size() > 1)
    {
        string strShellCommand;
        strShellCommand.clear();

        // Copy the shell command to the string to be passed to the process function
        size_t cmdsize = cmd.size() - 1;
        for (size_t i = 0; i < cmdsize; i++)
        {
            // Put spaces in between of the words
            if (i)
            {
                strShellCommand.push_back(' ');
            }

            // Copy the command words one by one
            strShellCommand.push_back(cmd.at(1 + i));
        }

        // Let the Shell module process the command
        Shell::process(strShellCommand);

        strShellCommand.dispose();
    }
    // Increments an integer variable
    else if (cmd.size() == 2 && cmd[1].compare("++"))
    {
        // Find the target variable
        DataType type;
        void* targetPtr = Program::findStorage(cmd.at(0), &type);

        if (targetPtr)
        {
            // Target variable is integer
            if (type == DataType::Integer)
            {
                // Increment the value of the variable
                (*(INTEGER*)targetPtr)++;
            }
            // Target variable is a real value
            else if (type == DataType::Real)
            {
                // Increase the value of the variable by 1.0
                (*(REAL*)targetPtr) += 1.0;
            }
            // Unexpected data type of the target variable
            else
            {
                Program::errorTypeUnexpected();
                return;
            }
        }
        else
        {
            return;
        }
    }
    // Decrements an integer variable
    else if (cmd.size() == 2 && cmd[1].compare("--"))
    {
        // Find the target variable
        DataType type;
        void* targetPtr = Program::findStorage(cmd.at(0), &type);

        if (targetPtr)
        {
            // Target variable is integer
            if (type == DataType::Integer)
            {
                // Decrement the value of the variable
                (*(INTEGER*)targetPtr)--;
            }
            // Target variable is a real value
            else if (type == DataType::Real)
            {
                // Decrease the value of the variable by 1.0
                (*(REAL*)targetPtr) -= 1.0;
            }
            // Unexpected data type of the target variable
            else
            {
                Program::errorTypeUnexpected();
                return;
            }
        }
        else
        {
            return;
        }
    }
    // Variable value assignment
    else if (cmd.size() >= 3 && cmd[1].contains('='))
    {
        // Find the variable in which the value is supposed to be stored
        DataType type;
        void* targetPtr = Program::findStorage(cmd.at(0), &type);

        // Check if the target variable exists
        if (!targetPtr)
        {
            return;
        }

        switch (type)
        {
            // Target variable is integer
            case DataType::Integer:
            {
                INTEGER oldValue = *(INTEGER*)targetPtr;

                // Resolve the operand
                INTEGER* operandPtr = Program::symbolMultiResolveInteger(cmd, 2);

                // Failed to resolve the operand
                if (!operandPtr)
                {
                    return;
                }

                INTEGER operandValue = *operandPtr;
                delete operandPtr;

                // Value assignment
                if (cmd[1].compare("="))
                {
                    (*(INTEGER*)targetPtr) = operandValue;
                }
                // Add operand value to itself
                else if (cmd[1].compare("+="))
                {
                    (*(INTEGER*)targetPtr) = oldValue + operandValue;
                }
                // Subtract operand value from itself
                else if (cmd[1].compare("-="))
                {
                    (*(INTEGER*)targetPtr) = oldValue - operandValue;
                }
                // Multiply itself by operand value
                else if (cmd[1].compare("*="))
                {
                    (*(INTEGER*)targetPtr) = oldValue * operandValue;
                }
                // Perform integer division by operand value on itself
                else if (cmd[1].compare("/="))
                {
                    if (operandValue == 0)
                    {
                        Program::errorDivisionByZero();
                        return;
                    }

                    (*(INTEGER*)targetPtr) = oldValue / operandValue;
                }
                // Set itself to the remained after integer division by the operand value
                else if (cmd[1].compare("%="))
                {
                    if (operandValue == 0)
                    {
                        Program::errorDivisionByZero();
                        return;
                    }

                    (*(INTEGER*)targetPtr) = oldValue % operandValue;
                }
                // Bit shift itself left
                else if (cmd[1].compare("<<="))
                {
                    (*(INTEGER*)targetPtr) = oldValue << operandValue;
                }
                // Bit shift itself right
                else if (cmd[1].compare(">>="))
                {
                    (*(INTEGER*)targetPtr) = oldValue >> operandValue;
                }
                // Self-cast bitwise AND
                else if (cmd[1].compare("&="))
                {
                    (*(INTEGER*)targetPtr) = oldValue & operandValue;
                }
                // Self-cast bitwise OR
                else if (cmd[1].compare("|="))
                {
                    (*(INTEGER*)targetPtr) = oldValue | operandValue;
                }
                // Self-cast bitwise XOR
                else if (cmd[1].compare("^="))
                {
                    (*(INTEGER*)targetPtr) = oldValue ^ operandValue;
                }
                // Not a valid integer operator
                else
                {
                    Program::errorOperatorInvalid(cmd[1]);
                    return;
                }

                break;
            }

            // Target variable is logical
            case DataType::Logical:
            {
                LOGICAL oldValue = *(LOGICAL*)targetPtr;

                // Resolve the operand
                LOGICAL* operandPtr = Program::symbolMultiResolveLogical(cmd, 2);

                // Failed to resolve the operand
                if (!operandPtr)
                {
                    return;
                }

                LOGICAL operandValue = *operandPtr;
                delete operandPtr;

                // Value assignment
                if (cmd[1].compare("="))
                {
                    (*(LOGICAL*)targetPtr) = operandValue;
                }
                // Self-cast AND
                else if (cmd[1].compare("&&="))
                {
                    (*(LOGICAL*)targetPtr) = oldValue && operandValue;
                }
                // Self-cast OR
                else if (cmd[1].compare("||="))
                {
                    (*(LOGICAL*)targetPtr) = oldValue || operandValue;
                }
                // Not a valid logical operator
                else
                {
                    Program::errorOperatorInvalid(cmd[1]);
                    return;
                }

                break;
            }

            // Target variable is real
            case DataType::Real:
            {
                REAL oldValue = *(REAL*)targetPtr;

                // Resolve the operand
                REAL* operandPtr = Program::symbolMultiResolveReal(cmd, 2);

                // Failed to resolve the operand
                if (!operandPtr)
                {
                    return;
                }

                REAL operandValue = *operandPtr;
                delete operandPtr;

                // Value assignment
                if (cmd[1].compare("="))
                {
                    (*(REAL*)targetPtr) = operandValue;
                }
                // Add operand value to itself
                else if (cmd[1].compare("+="))
                {
                    (*(REAL*)targetPtr) = oldValue + operandValue;
                }
                // Subtract operand value from itself
                else if (cmd[1].compare("-="))
                {
                    (*(REAL*)targetPtr) = oldValue - operandValue;
                }
                // Multiply itself by operand value
                else if (cmd[1].compare("*="))
                {
                    (*(REAL*)targetPtr) = oldValue * operandValue;
                }
                // Perform real number division by operand value on itself
                else if (cmd[1].compare("/="))
                {
                    if (operandValue == 0)
                    {
                        Program::errorDivisionByZero();
                        return;
                    }

                    (*(REAL*)targetPtr) = oldValue / operandValue;
                }
                // Not a valid real operator
                else
                {
                    Program::errorOperatorInvalid(cmd[1]);
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
