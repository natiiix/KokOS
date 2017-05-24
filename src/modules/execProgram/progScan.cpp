#include <modules/exec.hpp>
#include <c/stdio.h>
#include <kernel.h>

bool Program::scanThrough(void)
{
    size_t scanCounter = 0;
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

        // Check if the command on the line is valid
        if (cmd[0].compare("exit") ||
            cmd[0].compare("integer") ||
            cmd[0].compare("logical") ||
            cmd[0].compare("real") ||
            cmd[0].compare("push") ||
            cmd[0].compare("pop") ||
            cmd[0].compare("print") ||
            cmd[0].compare("if") ||
            cmd[0].compare("while") ||
            cmd[0].compare("else") ||
            cmd[0].compare("end") ||
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

        // Unable to recognize the command on this line
        // Print an error and the invalid line of code
        print("Unrecognized command on line ");
        printint(scanCounter);
        newline();
        // Convert words to a single string and print it
        string strCmd = string::join(cmd, ' ', false);
        sprint(strCmd);
        strCmd.dispose();
        newline();

        return false;
    }
    while (++scanCounter < programLen);

    // All commands are valid
    debug_print("progScan.cpp | Program::scanThrough() | No invalid command found!");
    return true;
}
