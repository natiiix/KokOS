#include <modules/exec.hpp>
#include <modules/shell_global.hpp>
#include <c/stdio.h>
#include <c/stdlib.h>
#include <drivers/storage/fat.h>
#include <kernel.h>

void Program::run(const char* const codePtr)
{
    string strCode;
    strCode.set(codePtr);

    vector<string> vectLines = strCode.split('\n', true);
    strCode.dispose();

    // Process the lines

    vectLines.dispose();
    m_variables.dispose();
    m_program.dispose();
}

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
                debug_print("exec.cpp | cmd_exec() | Failed to read the specified file!");
            }
        }
        else
        {
            debug_print("exec.cpp | cmd_exec() | Specified file is empty!");
        }

        delete file;
    }
    else
    {
        debug_print("exec.cpp | cmd_exec() | Specified file doesn't exist!");
    }

    vecArgs.dispose();
}

Variable::Variable(void)
{
    Name.clear();
    Pointer = nullptr;
}

void Variable::declare(const string& name, const DataType type)
{
    Name.clear();
    Name.push_back(name);

    Type = type;

    Pointer = nullptr;
}

void Variable::dispose(void)
{
    Name.dispose();
}
