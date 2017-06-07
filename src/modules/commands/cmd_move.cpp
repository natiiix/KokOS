#include <stddef.h>
#include <stdint.h>

#include <c/stdio.h>
#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#include <drivers/storage/fat.h>
#include <modules/shell_global.hpp>

#include <kernel.h>

void cmd_move(const string& strArgs)
{
    vector<string> vecArgs = strArgs.split(' ', true);
    
    if (vecArgs.size() != 2)
    {
        print("Invalid arguments!\n");
        print("Syntax: move <Source File Path> <Target File Path>\n");
        
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
        print("Invalid target directory path! Unable to move the file there!\n");

        delete fileSource;
        vecArgs.dispose();
        return;
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
        debug_print("cmd_move.cpp | cmd_move() | Source file contains some data!");

        // Create target file and copy the data from source file into target file
        fileTarget = writeFile(Shell::activePart, Shell::activeDir, vecArgs.at(1).c_str(), fileData, fileSource->size);
        delete fileData;
    }
    // File is empty
    else
    {
        debug_print("cmd_move.cpp | cmd_move() | Source file is empty!");

        // Source file is empty, using newFile() will be faster than using writeFile() to write an empty file
        fileTarget = newFile(Shell::activePart, Shell::activeDir, vecArgs.at(1).c_str(), 0);
    }

    // Check if the file copying / creation was successful
    if (fileTarget)
    {
        debug_print("cmd_move.cpp | cmd_move() | File has been copied successfully!");
        delete fileTarget;

        // File has been copied successfully, now it's time to delete the old source file
        if (deleteEntry(Shell::activePart, Shell::activeDir, vecArgs.at(0).c_str()))
        {
            debug_print("cmd_move.cpp | cmd_move() | Entry has been deleted successfully!");
        }
        else
        {
            debug_print("cmd_move.cpp | cmd_move() | Failed to delete the entry!");
        }
    }
    else
    {
        debug_print("cmd_move.cpp | cmd_move() | Failed to copy the file!");
    }

    delete fileSource;

    vecArgs.dispose();
}
