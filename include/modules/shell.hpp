#pragma once

#include <cpp/string.hpp>

#if defined(__cplusplus)
extern "C"
{
#endif

void shell_init(void);

#if defined(__cplusplus)
}
#endif

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
