#include <stddef.h>
#include <stdint.h>

#include <c/stdio.h>
#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#include <drivers/storage/fat.h>
#include <modules/shell_global.hpp>

#include <kernel.h>

void cmd_copy(const string& strArgs)
{
    vector<string> vecArgs = strArgs.split(' ', true);
    
    if (vecArgs.size() != 2)
    {
        print("Invalid arguments!\n");
        print("Syntax: copy <Source File Path> <Target File Path>\n");
        
        vecArgs.dispose();
        return;
    }

    // Find the source file
    struct FILE* fileSource = getFile(Shell::activePart, Shell::activeDir, vecArgs.at(0).c_str());

    // Source file doesn't exist
    if (!fileSource)
    {
        print("Source file doesn't exist!\n");
        
        vecArgs.dispose();
        return;
    }

    // Make sure the source file path is inside an existing directory
    if (!dirPathValid(Shell::activePart, Shell::activeDir, vecArgs.at(1).c_str()))
    {
        print("Invalid target directory path!\n");
    }

    // Check if the target file exists
    struct FILE* fileTarget = getFile(Shell::activePart, Shell::activeDir, vecArgs.at(1).c_str());

    // Target file already exists
    if (fileTarget)
    {
        print("Target file already exists!\n");
        
        delete fileSource;
        delete fileTarget;

        vecArgs.dispose();
        return;
    }

    // Read data from the source file
    uint8_t* fileData = readFile(fileSource);

    // File is NOT empty
    if (fileData)
    {
        debug_print("cmd_copy.cpp | cmd_copy() | Source file contains some data!");

        // Create target file and copy the data from source file into target file
        fileTarget = writeFile(Shell::activePart, Shell::activeDir, vecArgs.at(1).c_str(), fileData, fileSource->size);
        delete fileData;
    }
    // File is empty
    else
    {
        debug_print("cmd_copy.cpp | cmd_copy() | Source file is empty!");

        // Source file is empty, using newFile() will be faster than using writeFile() to write an empty file
        fileTarget = newFile(Shell::activePart, Shell::activeDir, vecArgs.at(1).c_str(), 0);
    }

    // Check if the file copying / creation was successful
    if (fileTarget)
    {
        debug_print("cmd_copy.cpp | cmd_copy() | File has been copied successfully!");
        delete fileTarget;
    }
    else
    {
        debug_print("cmd_copy.cpp | cmd_copy() | Failed to copy the file!");
    }

    delete fileSource;

    vecArgs.dispose();
}
