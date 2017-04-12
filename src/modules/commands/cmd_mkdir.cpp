#include <c/stdio.h>

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#include <drivers/storage/fat.h>
#include <modules/shell_global.hpp>

#include <kernel.h>

void cmd_mkdir(const string& strArgs)
{
    vector<string> vecArgs = strArgs.split(' ', true);
    
    if (vecArgs.size() != 1)
    {
        print("Invalid arguments!\n");
        print("Syntax: mkdir <Directory Path>\n");
        
        vecArgs.dispose();
        return;
    }

    struct FILE* dir = newDir(activePart, activeDir, vecArgs[0].c_str());

    if (dir)
    {
        debug_print("cmd_mkdir.cpp | cmd_mkdir() | Directory has been created successfully!");
        delete dir;
    }
    else
    {
        debug_print("cmd_mkdir.cpp | cmd_mkdir() | Failed to create the directory!");
    }

    vecArgs.dispose();
}