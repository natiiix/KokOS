#include <modules/exec.hpp>
#include <modules/shell_global.hpp>
#include <c/stdio.h>
#include <drivers/storage/fat.h>

void cmd_exec(const string& strArgs)
{
    vector<string> vecArgs = strArgs.split(' ', true);
    
    if (vecArgs.size() != 1)
    {
        print("Invalid arguments!\n");
        print("Syntax: exec <File Path>\n");
        
        vecArgs.dispose();
        return;
    }

    struct FILE* file;

    // Find the file
    file = getFile(Shell::activePart, Shell::activeDir, vecArgs.at(0).c_str());

    // File exists
    if (file)
    {
        if (file->size)
        {
            // Read file's content
            char* content = (char*)readFile(file);

            if (content)
            {
                Program prog;
                prog.run(content);
                delete content;
            }
            else
            {
                print("Failed to read the specified file!");
            }
        }
        else
        {
            print("Cannot execute an empty file!");
        }

        delete file;
    }
    else
    {
        print("File doesn't exist!");
    }

    vecArgs.dispose();
}
