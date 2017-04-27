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
    void* Pointer; // pointer to the value of the variable

    Variable(void);
    void declare(const string& name, const DataType type);
    void dispose(void);
};

class Program
{
public:
    void run(const char* const codePtr);
private:
    vector<uint32_t*> m_program;
    vector<Variable> m_variables;
};
