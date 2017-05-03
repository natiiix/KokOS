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

        case DataType::Real:
            Pointer = malloc(sizeof(REAL));
            (*(REAL*)Pointer) = 0.0;
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
