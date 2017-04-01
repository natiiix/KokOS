#pragma once

#include <cpp/string.hpp>

class Shell
{
public:
    void init(void);

private:
    string strPrefix;

    // Internal shell calls
    string readline(void);

    // Helping functions
    char* _generate_spaces(const size_t count);
    void _input_update(void);
    void _input_clean(void);
};
