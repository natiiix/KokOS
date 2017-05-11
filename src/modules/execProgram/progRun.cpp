#include <modules/exec.hpp>
#include <c/stdio.h>

void Program::run(const char* const codePtr)
{
    string strCode;
    strCode.set(codePtr);

    vector<string> vectLines = strCode.split('\n', false);
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
    
    // Execute commands one after another until the program finishes
    while (m_counter < m_program.size())
    {
        Program::executeCommand();
    }

    // After the program execution is done the scope level should be 0
    // This doesn't apply when the program is terminated via Program::exit()
    if (m_scope && m_counter != PROGRAM_COUNTER_EXIT)
    {
        Program::error("End of scope expected!");
    }
    
    // Dispose the whole program code
    while (m_program.size())
    {
        m_program.back().dispose();
        m_program.pop_back();
    }
    m_program.dispose();

    // Dispose all variables
    while (m_variables.size())
    {
        m_variables.back().dispose();
        m_variables.pop_back();
    }
    m_variables.dispose();

    m_scopeStack.dispose();
}
