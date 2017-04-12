#include <c/stdio.h>

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#include <drivers/storage/fat.h>
#include <modules/shell_global.hpp>

#include <kernel.h>

void cmd_delete(const string& strArgs)
{
    vector<string> vecArgs = strArgs.split(' ', true);
    
    if (vecArgs.size() != 1)
    {
        print("Invalid arguments!\n");
        print("Syntax: delete <File/Directory Path>\n");
        
        vecArgs.dispose();
        return;
    }

    if (deleteEntry(activePart, activeDir, vecArgs[0].c_str()))
    {
        debug_print("cmd_delete.cpp | cmd_delete() | Entry has been deleted successfully!");
    }
    else
    {
        debug_print("cmd_delete.cpp | cmd_delete() | Failed to delete the entry!");
    }

    vecArgs.dispose();
}