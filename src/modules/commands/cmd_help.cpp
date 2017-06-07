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
    print("<Partition Letter>: - Changes active partition\n");
    print("cd <Directory Path> - Changes active directory\n");
    print("clear - Clears the terminal screen\n");
    print("color <Background Color> <Foreground Color> - Changes color scheme of shell\n");
    print("copy <Source File Path> <Target File Path> - Copies source file to target path\n");
    print("delete <File/Directory Path> - Deletes file or directory\n");
    print("dir [Directory Path] - Displays content of directory\n");
    print("disk <Argument> - Prints disk-related information\n");
    print("exec <File Path> - Executes program file\n");
    print("help - Displays available commands and their syntax\n");
    print("mkdir <Directory Path> - Creates new directory\n");
    print("mkfile <File Path> - Creates new file\n");
    print("move <Source File Path> <Target File Path> - Moves source file to target path\n");
    print("rename <Old Name> <New Name> - Renames file or directory\n");
    print("text <File Path> - Opens or creates file for text editing\n");

    vecArgs.dispose();
}