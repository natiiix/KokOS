#include <modules/exec.hpp>
#include <c/stdio.h>

void Program::error(const char* const str)
{
    print("Error on line ");
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

void Program::errorTypesIncompatible(void)
{
    Program::error("Variables do not have matching data types!");
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
