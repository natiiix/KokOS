#pragma once

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#include "shell_global.hpp"

namespace Shell
{
    void process(const string& strInput);

    // Internal shell calls
    void initModules(void);
    string readline(void);

    // Helping functions
    char* _generate_spaces(const size_t count);
    void _input_update(void);
    void _input_clean(void);
    void _update_prefix(void);
}
