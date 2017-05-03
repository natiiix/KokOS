#pragma once

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

typedef int32_t INTEGER;
typedef double REAL;
typedef bool LOGICAL;

static const size_t PROGRAM_COUNTER_EXIT = (size_t)-1;

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

    void executeCommand(void); // executes the command at the current line

    void varDeclare(const string& name, const DataType type);
    Variable* varFind(const string& name);
    bool varNameIsKeyword(const string& name); // check variable name against a list of keywords
    bool varNameInvalidChar(const string& name); // check if variable name consists of valid characters only

    void exit(void); // stops the program execution
    void error(const char* const str); // prints an error message and exits the program
    void error(const string& str);
};
