#include <modules/exec.hpp>
#include <c/stdio.h>

void Program::error(const char* const str)
{
    print("Runtime error on line ");
    printint(m_counter);
    newline();
    print(str);
    newline();

    m_counter = PROGRAM_COUNTER_EXIT;
}

void Program::error(const string& str)
{
    Program::error(str.c_str());
}

void Program::errorVarUndeclared(const string& strVarName)
{
    string strErrorMsg;
    strErrorMsg.clear();
    
    strErrorMsg.push_back("Variable \"");
    strErrorMsg.push_back(strVarName);
    strErrorMsg.push_back("\" has not been declared in current scope!");

    Program::error(strErrorMsg);
    strErrorMsg.dispose();
}

void Program::errorSymbolUnresolved(const string& strSymbol)
{
    string strError;
    strError.clear();

    strError.push_back("Unable to resolve symbol \"");
    strError.push_back(strSymbol);
    strError.push_back("\"!");

    Program::error(strError);
    strError.dispose();
}

void Program::errorTypeUnexpected(void)
{
    Program::error("Unexpected data type!");
}

void Program::errorOperatorInvalid(const string& strOperator)
{
    string strError;
    strError.clear();

    strError.push_back("Symbol \"");
    strError.push_back(strOperator);
    strError.push_back("\" is not a valid operator!");

    Program::error(strError);
    strError.dispose();
}

void Program::errorDivisionByZero(void)
{
    Program::error("Cannot divide by zero!");
}

void Program::errorScan(const size_t index)
{
    print("Error on line ");
    // Print the line index
    printint(index);
    newline();
    
    // Only print the content of the line if it's within the program
    // If an error happens after the program is done executing, the line index is outside of the program
    if (index < m_program.size())
    {
        // Print the line's content
        string strLine = string::join(m_program.at(index), ' ');
        print("\"");
        sprint(strLine);
        print("\"");

        strLine.dispose();
        newline();
    }
}

void Program::errorScan(const size_t index, const char* const message)
{
    Program::errorScan(index);
    
    // Print the error message
    print(message);
    newline();
}

void Program::errorSubUndefined(const string& strSubName)
{
    string strErrorMsg;
    strErrorMsg.clear();
    
    strErrorMsg.push_back("Subroutine \"");
    strErrorMsg.push_back(strSubName);
    strErrorMsg.push_back("\" is undefined!");

    Program::error(strErrorMsg);
    strErrorMsg.dispose();
}
