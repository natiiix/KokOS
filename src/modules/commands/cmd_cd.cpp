#include <c/stdio.h>

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#include <drivers/storage/fat.h>
#include <modules/shell.hpp>

void cmd_cd(const string& strArgs)
{
    vector<string> vecArgs = strArgs.split(' ', true);
    
    if (vecArgs.size() != 1)
    {
        print("Invalid arguments!\n");
        print("Syntax: cd <Directory Path>\n");
        
        vecArgs.dispose();
        return;
    }

    uint32_t newDir = resolvePath(Shell::activePart, Shell::activeDir, vecArgs[0].c_str());

    if (newDir)
    {
        // If the directory path is absolute delete the old path
        if (vecArgs[0][0] == '/')
        {
            Shell::pathStructure.clear();
        }

        Shell::activeDir = newDir;

        vector<string> pathElements = vecArgs[0].split('/', true);
        
        for (size_t i = 0; i < pathElements.size(); i++)
        {
            // Ignore self-pointing path elements
            if (pathElements[i] == ".")
            {
                continue;
            }
            // When going one directory up remove the last directory from the vector
            else if (pathElements[i] == "..")
            {
                Shell::pathStructure.pop_back();
            }
            // When going one directory down append the directory to the vector
            else
            {
                // The directory name string must not be disposed
                // Therefore we need to copy it into a separate string outside the vector
                // This is done automatically when converting the name to uppercase
                Shell::pathStructure.push_back(pathElements[i].tolower());
            }
        }

        pathElements.dispose();

        Shell::_update_prefix();
    }
    else
    {
        print("Invalid directory path!\n");
    }

    vecArgs.dispose();
}