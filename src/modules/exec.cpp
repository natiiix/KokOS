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

    m_program.clear();

    for (size_t i = 0; i < vectLines.size(); i++)
    {
        // Split each line into space separated words
        m_program.push_back(vectLines[i].split(' ', true));
    }

    vectLines.dispose();

    // Set program counter and scope depth
    m_counter = 0;
    m_scope = 0;

    // Initial main program scope push
    Program::scopePush();
    
    // Execute commands one after another until the program finishes
    while (m_counter < m_program.size())
    {
        Program::executeCommand();
    }

    // Scope pop at the end of the main program
    Program::scopePop();
    
    // Dispose the whole program code
    while (m_program.size())
    {
        m_program.back().dispose();
        m_program.pop_back();
    }
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
        // If a variable has a lower scope level than the one that's being popped
        // there shouldn't be any more variables declared in this scope before it
        if (m_variables[varsize - 1 - i].Scope < m_scope)
        {
            break;
        }

        // Dispose the variable and pop it from the vector
        m_variables.back().dispose();
        m_variables.pop_back();
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

void Program::executeCommand(void)
{
    // Ignore empty lines and comments
    if (!m_program[m_counter].size() ||
        (m_program[m_counter][0].size() >= 2 && m_program[m_counter][0][0] == '/' && m_program[m_counter][0][1] == '/'))
    {
        m_counter++;
        return;
    }

    // I figured out that a shortcut for the current command might be useful
    vector<string>& cmd = m_program[m_counter];

    if (cmd[0].compare("exit") && cmd.size() == 1)
    {
        Program::exit();
    }
    else if (cmd[0].compare("integer") && cmd.size() == 2)
    {
        Program::varDeclare(cmd[1], DataType::Integer);
    }
    else
    {
        print("Unexpected command on line ");

        printint(m_counter);
        print(": \"");
        
        string strCmd = string::join(cmd, ' ', true);
        print(strCmd.c_str());
        strCmd.dispose();

        print("\"\n");

        Program::exit();
    }

    // If the execution was successful and program exit wasn't requested
    if (m_counter != PROGRAM_COUNTER_EXIT)
    {
        // Progress to next command
        m_counter++;
    }
}

void Program::varDeclare(const string& name, const DataType type)
{
    if (Program::varFind(name))
    {
        print("Variable with this name is already declared!\n");
        
        Program::exit();
        return;
    }

    m_variables.push_back(Variable());
    m_variables.back().declare(name, type, m_scope);
}

Variable* Program::varFind(const string& name)
{
    size_t varsize = m_variables.size();
    
    for (size_t i = 0; i < varsize; i++)
    {
        // Find the variable with the specified name
        if (m_variables[i].Name.compare(name))
        {
            // Return pointer to the variable
            return &m_variables[i];
        }
    }

    // Return nullptr if there is no variable with the specified name
    return nullptr;
}

void Program::exit(void)
{
    m_counter = PROGRAM_COUNTER_EXIT;
}
