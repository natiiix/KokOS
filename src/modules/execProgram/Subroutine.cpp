#include <modules/exec.hpp>
#include <c/stdlib.h>
#include <kernel.h>

void Subroutine::define(const string& name, const size_t counter)
{
    Name.clear();
    Name.push_back(name);

    Counter = counter;
}
