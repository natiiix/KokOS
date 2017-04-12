#include <c/stdio.h>

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

void cmd_help(const string& strArgs)
{
    vector<string> vecArgs = strArgs.split(' ', true);
    
    if (vecArgs.size() != 0)
    {
        print("Invalid arguments!\n");
        print("Syntax: help\n");
        
        vecArgs.dispose();
        return;
    }

    print("COMMAND <REQUIRED ARGUMENT> [OPTIONAL ARGUMENT]\n");
    print("help - Displays available commands and their syntax\n");
    print("clear - Clears the terminal screen\n");
    print("<Partition Letter>: - Changes active partition\n");
    print("cd <Directory Path> - Changes active directory\n");
    print("dir [Directory Path] - Displays content of a directory\n");
    print("mkfile <File Path> - Creates a new file\n");
    print("mkdir <Directory Path> - Creates a new directory\n");
    print("delete <File/Directory Path> - Deletes a file/directory\n");
    print("disk <Action> <Arguments> - Performs a disk-related operation\n");

    vecArgs.dispose();
}