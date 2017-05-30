#include <modules/exec.hpp>
#include <c/stdio.h>
#include <c/stdlib.h>
#include <kernel.h>

void Array::declare(const string& name, const DataType type, const size_t scope, const size_t length)
{
    Name.set(name);
    Type = type;
    Scope = scope;
    Length = length;

    // Zero length array
    if (!length)
    {
        Pointer = nullptr;
        return;
    }

    switch (type)
    {
        case DataType::Integer:
            Pointer = malloc(sizeof(INTEGER) * length);            
            for (size_t i = 0; i < length; i++)
            {
                ((INTEGER*)Pointer)[i] = 0;
            }
            break;

        case DataType::Logical:
            Pointer = malloc(sizeof(LOGICAL) * length);
            for (size_t i = 0; i < length; i++)
            {
                ((LOGICAL*)Pointer)[i] = false;
            }
            break;

        case DataType::Real:
            Pointer = malloc(sizeof(REAL) * length);            
            for (size_t i = 0; i < length; i++)
            {
                ((REAL*)Pointer)[i] = 0.0;
            }
            break;

        default:
            debug_print("Array.cpp | Array::declare() | Invalid array data type!");
            Pointer = nullptr;
            break;
    }
}

void Array::dispose(void)
{
    Name.dispose();

    if (Pointer)
    {
        free(Pointer);
    }
}

void* Array::getElementPtr(const size_t index)
{
    // Index is within the array boundaries
    if (index < Array::Length)
    {
        switch (Array::Type)
        {
            case DataType::Integer:
                return &((INTEGER*)Pointer)[index];

            case DataType::Logical:
                return &((LOGICAL*)Pointer)[index];

            case DataType::Real:
                return &((REAL*)Pointer)[index];

            default:
                break;
        }
    }
    
    // Index out of array boundaries
    return nullptr;
}
