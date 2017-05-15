#pragma once

#include <cpp/vector.hpp>

#include "shell_global.hpp"

class string;

namespace Shell
{
    // Used by cmd_cd()
    extern vector<string> pathStructure;

    // The maximum amount of command strings saved in history
    static const uint8_t HISTORY_LIMIT = 0x10;
    // This value is present in historyIdx whenever history isn't being browsed
    static const uint8_t HISTORY_INDEX_DEFAULT = 0xFF;

    void process(const string& strInput);

    // Internal shell calls
    void initModules(void);
    string readline(void);

    // Helping functions
    void _input_update(void);
    void _input_clean(void);
    void _update_prefix(void);
}
