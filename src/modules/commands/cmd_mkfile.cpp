#include <c/stdio.h>

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#include <drivers/storage/fat.h>
#include <modules/shell_global.hpp>

#include <kernel.h>

void cmd_mkfile(const string& strArgs)
{
    vector<string> vecArgs = strArgs.split(' ', true);
    
    if (vecArgs.size() != 1)
    {
        print("Invalid arguments!\n");
        print("Syntax: mkfile <File Path>\n");
        
        vecArgs.dispose();
        return;
    }

    struct FILE* file = newFile(activePart, activeDir, vecArgs[0].c_str(), 0);

    if (file)
    {
        debug_print("cmd_mkfile.cpp | cmd_mkfile() | File has been created successfully!");
        delete file;
    }
    else
    {
        debug_print("cmd_mkfile.cpp | cmd_mkfile() | Failed to create the file!");
    }

    vecArgs.dispose();
}