// This file is not supposed to be compiled
// It's just a template to speed up making of new commands
#define CMD_PROTOTYPE
#ifndef CMD_PROTOTYPE

#include <c/stdio.h>

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

void cmd_prototype(const string& strArgs)
{
    vector<string> vecArgs = strArgs.split(' ', true);
    
    if (vecArgs.size() != 0)
    {
        print("Invalid arguments!\n");
        print("Syntax: prototype\n");
        
        vecArgs.dispose();
        return;
    }

    // COMMAND CODE GOES HERE

    vecArgs.dispose();
}

#endif
