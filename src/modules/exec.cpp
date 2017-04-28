#include <modules/exec.hpp>
#include <modules/shell_global.hpp>
#include <c/stdio.h>
#include <c/stdlib.h>
#include <drivers/storage/fat.h>
#include <kernel.h>

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

// class Variable
void Variable::declare(const string& name, const DataType type, const size_t scope)
{
    Name.clear();
    Name.push_back(name);

    Type = type;
    Scope = scope;

    Pointer = nullptr;
}

void Variable::dispose(void)
{
    Name.dispose();
}

// class Program
void Program::run(const char* const codePtr)
{
    string strCode;
    strCode.set(codePtr);

    vector<string> vectLines = strCode.split('\n', true);
    strCode.dispose();

    // TODO: Process the lines into program instructions

    vectLines.dispose();

    // Set program counter and scope depth
    m_counter = 0;
    m_scope = 0;

    // TODO: Program execution

    m_program.dispose();
    m_variables.dispose();

    m_scopeStack.dispose();
}

void Program::scopePush(void)
{
    m_scopeStack.push_back(m_counter);
    m_scope++;
}

void Program::scopePop(void)
{
    // Dispose all variables declared inside of this scope level
    size_t varsize = m_variables.size();
    for (size_t i = 0; i < varsize; i++)
    {
        // Find the first variable declared on the current scope level
        if (m_variables[i].Scope == m_scope)
        {
            // All variables declared after the first one necessarily belong to the same scope
            // And all of them need to be disposed because they shouldn't exist outside of the the scope
            m_variables.pop_back(varsize - i);
            break;
        }
    }

    if (m_scope)
    {
        m_scope--;
        m_scopeStack.pop_back();
    }
    else
    {
        debug_print("exec.cpp | Program::scopePop() | Cannot perform scope pop at 0 scope depth!");
    }
}
