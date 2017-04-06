#pragma once

#include <cpp/string.hpp>
#include <cpp/vector.hpp>

#include <modules/module.hpp>

class Shell
{
public:
    void init(void);
    void process(const string& strInput);

private:
    string m_strPrefix;

    // Modules
    vector<Module> m_modules;

    // Internal shell calls
    void initModules(void);
    string readline(void);

    // Helping functions
    char* _generate_spaces(const size_t count);
    void _input_update(void);
    void _input_clean(void);
};
