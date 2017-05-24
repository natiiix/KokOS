#include <modules/exec.hpp>
#include <c/stdio.h>
#include <kernel.h>

bool Program::scanThrough(void)
{
    size_t scanCounter = 0;
    size_t scopeLevel = 0;
    size_t programLen = m_program.size();

    // Runs through the code line by line
    do
    {
        // I figured out that a shortcut for the current command might be useful
        vector<string>& cmd = m_program[scanCounter];

        // Ignore empty lines and comments
        if (!cmd.size() || (cmd[0].size() >= 2 && cmd[0][0] == '/' && cmd[0][1] == '/'))
        {
            continue;
        }

        // Subroutine found
        if (cmd[0].compare("sub") && cmd.size() == 2)
        {
            debug_print("progScan.cpp | Program::scanThrough() | Subroutine definiton found!");
            
            if (Program::subDefine(cmd[1], scanCounter))
            {
                // Subroutine was successfully defined
                continue;
            }
            else
            {
                // Failed to define the subroutine
                return false;
            }
        }

        // Check if the command on the line is valid
        if (cmd[0].compare("exit") ||
            cmd[0].compare("integer") ||
            cmd[0].compare("logical") ||
            cmd[0].compare("real") ||
            cmd[0].compare("print") ||
            cmd[0].compare("else") ||
            cmd[0].compare("break") ||
            cmd[0].compare("continue") ||
            cmd[0].compare("echo") ||
            cmd[0].compare("read") ||
            (cmd.size() == 2 && cmd[1].compare("++")) || // variable incrementation
            (cmd.size() == 2 && cmd[1].compare("--")) || // variable decrementation
            (cmd.size() >= 3 && cmd.size() <= 5 && cmd[1].contains("="))) // variable assignment
        {
            continue;
        }
        // Beginning of a new scope
        else if (cmd[0].compare("push") || cmd[0].compare("if") || cmd[0].compare("while"))
        {
            // Increment the scope level depth
            scopeLevel++;
        }
        // End of a scope
        else if (cmd[0].compare("pop") || cmd[0].compare("end"))
        {
            if (scopeLevel)
            {
                // Decremenet the scope level depth
                scopeLevel--;
            }
            // End statement cannot be performed at 0 scope level because there is no scope to be popped
            else
            {
                Program::errorScan(scanCounter, "Unexpected end of scope!");
            }
        }

        // Unable to recognize the command on this line
        // Print an error and the invalid line of code
        Program::errorScan(scanCounter, "Unable to recognize the command!");
        return false;
    }
    while (++scanCounter < programLen);

    // The program should end with scope depth level 0 (the program base scope)
    if (scopeLevel)
    {
        Program::errorScan(scanCounter, "Unexpected end of program! Expected an end of scope!");
        return false;
    }

    // All commands are valid
    debug_print("progScan.cpp | Program::scanThrough() | No invalid command found!");
    return true;
}
