#include <modules/exec.hpp>
#include <c/stdio.h>
#include <c/stdlib.h>
#include <kernel.h>

void Variable::declare(const string& name, const DataType type, const size_t scope)
{
    Name.clear();
    Name.push_back(name);

    Type = type;
    Scope = scope;

    switch (type)
    {
        case DataType::Integer:
            Pointer = malloc(sizeof(INTEGER));
            (*(INTEGER*)Pointer) = 0;
            break;

        case DataType::Logical:
            Pointer = malloc(sizeof(LOGICAL));
            (*(LOGICAL*)Pointer) = false;
            break;

        default:
            debug_print("exec_Variable.cpp | Variable::declare() | Invalid variable data type!");
            Pointer = nullptr;
            break;
    }
}

void Variable::dispose(void)
{
    Name.dispose();
    free(Pointer);
}

INTEGER Variable::getInteger(void) const
{
    if (Variable::Type == DataType::Integer)
    {
        return *(INTEGER*)Variable::Pointer;
    }
    else
    {
        debug_print("exec_Variable.cpp | Variable::getInteger() | Variable was not declared as integer!");
        return 0;
    }
}

LOGICAL Variable::getLogical(void) const
{
    if (Variable::Type == DataType::Logical)
    {
        return *(LOGICAL*)Variable::Pointer;
    }
    else
    {
        debug_print("exec_Variable.cpp | Variable::getLogical() | Variable was not declared as logical!");
        return false;
    }
}

REAL Variable::getReal(void) const
{
    if (Variable::Type == DataType::Real)
    {
        return *(REAL*)Variable::Pointer;
    }
    else
    {
        debug_print("exec_Variable.cpp | Variable::getReal() | Variable was not declared as real!");
        return false;
    }
}

void Variable::set(const INTEGER value)
{
    if (Variable::Type == DataType::Integer)
    {        
        (*(INTEGER*)Variable::Pointer) = value;
    }
    else
    {
        debug_print("exec_Variable.cpp | Variable::set() | Variable was not declared as integer!");
    }
}

void Variable::set(const LOGICAL value)
{
    if (Variable::Type == DataType::Logical)
    {        
        (*(LOGICAL*)Variable::Pointer) = value;
    }
    else
    {
        debug_print("exec_Variable.cpp | Variable::set() | Variable was not declared as logical!");
    }
}

void Variable::set(const REAL value)
{
    if (Variable::Type == DataType::Real)
    {        
        (*(REAL*)Variable::Pointer) = value;
    }
    else
    {
        debug_print("exec_Variable.cpp | Variable::set() | Variable was not declared as real!");
    }
}
