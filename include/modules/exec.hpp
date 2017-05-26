#pragma once

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

typedef int32_t INTEGER;
typedef bool LOGICAL;
typedef double REAL;

static const size_t PROGRAM_COUNTER_EXIT = ~((size_t)0);

enum PROGRAM_NAME
{
    PROGRAM_NAME_OK,
    PROGRAM_NAME_INVALID_CHAR,
    PROGRAM_NAME_KEYWORD,
};

enum DataType
{
    Integer,
    Logical,
    Real,
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

    INTEGER getInteger(void) const;
    LOGICAL getLogical(void) const;
    REAL getReal(void) const;

    void set(const INTEGER value);
    void set(const LOGICAL value);
    void set(const REAL value);
};

class Subroutine
{
public:
    string Name;
    size_t Counter;

    void define(const string& name, const size_t counter);
};

class Array
{
public:
    string Name; // name of the array
    DataType Type; // data type of the array
    size_t Scope; // scope level depth at which the array was declared
    size_t Length; // number of elements in the array
    void* Pointer; // pointer to the first element in the array

    void declare(const string& name, const DataType type, const size_t scope, const size_t length);
    void dispose(void);

    void* getElementPtr(const size_t index); // returns pointer to element with specified index
};

class Program
{
public:
    // ---- progRun ----
    void run(const char* const codePtr);

private:
    vector<vector<string>> m_program; // lines of code
    vector<Variable> m_variables; // vector of all currently declared variables
    vector<Subroutine> m_subroutines; // vector of all subroutines found in the code
    vector<Array> m_arrays; // vector of all currently declared arrays

    size_t m_counter; // program counter
    size_t m_scope; // current scope depth
    vector<size_t> m_scopeStack; // contains indexes at which the program pushed the scope (used by while loop)

    // ---- progCmdExe ----
    void executeCommand(void); // executes the command at the current line

    // ---- progScan ----
    bool scanThrough(void); // runs through the code, tries to find invalid commands (true = OK, false = Error)

    // ---- progVar ----
    bool varDeclare(const string& name, const DataType type); // declares a new variable with a specified name and data type
    Variable* varFind(const string& name); // finds a variable by name and returns a pointer to it, returns nullptr on failure
    INTEGER* varGetIntegerPtr(const string& varName); // returns an integer value pointer
    LOGICAL* varGetLogicalPtr(const string& varName); // returns a logical value pointer
    REAL* varGetRealPtr(const string& varName); // returns a real value pointer
    enum PROGRAM_NAME nameValid(const string& name); // check if a name is valid (contains only valid characters and isn't a keyword)
    void* valueCopy(const DataType type, const void* const source); // stores a value of specified data type in persistent memory space

    // ---- progSub ----
    bool subDefine(const string& name, const size_t counter); // defines a new subroutine if possible
    size_t subFind(const string& name); // returns counter index of a subroutine, return exit counter value if the subroutine doesn't exist

    // ---- progError ----
    void error(const char* const str); // prints an error message and exits the program
    void error(const string& str); // overload of error() to make it easier to use string objects
    void errorVarUndeclared(const string& name); // "variable hasn't been declared" error
    void errorSymbolUnresolved(const string& name); // "unable to resolve symbol" error
    void errorTypeUnexpected(void); // "variables don't have matching data types" error
    void errorOperatorInvalid(const string& strOperator); // "symbol is not a valid operator" error
    void errorDivisionByZero(void); // "cannot divide by zero" error
    void errorScan(const size_t index); // error during pre-execution scan
    void errorScan(const size_t index, const char* const message); // same as above with an extra error message
    void errorSubUndefined(const string& strSubName);

    // ---- progExpression ----
    bool symbolToInteger(const string& strSymbol, INTEGER* const output, const bool throwError = true);
    bool symbolToLogical(const string& strSymbol, LOGICAL* const output, const bool throwError = true);
    bool symbolToReal(const string& strSymbol, REAL* const output, const bool throwError = true);

    bool evaluateInteger(const string& strSymbol1, const string& strOperator, const string& strSymbol2, INTEGER* const output);
    bool evaluateLogical(const string& strSymbol1, const string& strOperator, const string& strSymbol2, LOGICAL* const output);
    bool evaluateReal(const string& strSymbol1, const string& strOperator, const string& strSymbol2, REAL* const output);

    bool realSqrt(const string& strSourceSymbol, Variable* const outputVariable);
    bool convertToInteger(const string& strSourceSymbol, Variable* const outputVariable);
    bool convertToLogical(const string& strSourceSymbol, Variable* const outputVariable);
    bool convertToReal(const string& strSourceSymbol, Variable* const outputVariable);

    // Extracts array information from a symbol if possible
    bool symbolToArrayInfo(const string& strSymbol, string& outName, size_t* const outAccessor);
    // Returns a pointer to the value represented by the symbol represents and determines the data type of the value
    void* symbolToValue(const string& strSymbol, DataType* const outType);
    // Resolves one or more symbols and performs an operation depending on the symbols
    void* symbolMultiResolve(const vector<string> vectSymbols, const size_t firstIndex, DataType* const outType);

    // ---- progScope ----
    void scopePush(void); // pushes current program counter onto the scope stack and incremenets the scope level
    void scopePop(void); // decrements the scope level and disposes all variables that were declared in that scope

    size_t findEnd(void);
    size_t findElse(void);

    void elseLoop(void);
    bool breakScope(const size_t levelsToBreak, const bool breakLast);
    
    // ---- progArray ----
    bool arrayDeclare(const string& name, const DataType type, const size_t length); // declares a new array at current scope
    Array* arrayFind(const string& name); // finds an array by its name returns pointer to it
};
