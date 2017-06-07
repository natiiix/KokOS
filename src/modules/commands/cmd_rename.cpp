#include <c/stdio.h>

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#include <drivers/storage/fat.h>
#include <modules/shell_global.hpp>

#include <kernel.h>

void cmd_rename(const string& strArgs)
{
    vector<string> vecArgs = strArgs.split(' ', true);
    
    if (vecArgs.size() != 2)
    {
        print("Invalid arguments!\n");
        print("Syntax: rename <Old Name> <New Name>\n");
        
        vecArgs.dispose();
        return;
    }

    if (renameEntry(Shell::activePart, Shell::activeDir, vecArgs[0].c_str(), vecArgs[1].c_str()))
    {
        debug_print("cmd_rename.cpp | cmd_rename() | Entry has been successfully renamed!");
    }
    else
    {
        debug_print("cmd_rename.cpp | cmd_rename() | Failed to rename the entry!");
    }

    vecArgs.dispose();
}