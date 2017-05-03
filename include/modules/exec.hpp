#pragma once

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

enum DataType
{
    Integer,
    Real,
    Logical,
};

class Variable
{
public:
    string Name; // symbol name representing the variable
    DataType Type; // data type of the variable
    size_t Scope; // scope depth at which this variable was declared
    void* Pointer; // pointer to the value of the variable

    void declare(const string& name, const DataType type, const size_t scope);
    void dispose(void);
};

class Program
{
public:
    void run(const char* const codePtr);

private:
    vector<vector<string>> m_program; // lines of code
    vector<Variable> m_variables;

    size_t m_counter; // program counter
    size_t m_scope; // current scope depth
    vector<size_t> m_scopeStack; // contains indexes at which the program pushed the scope (to be used by loops)

    void scopePush(void);
    void scopePop(void);
};
