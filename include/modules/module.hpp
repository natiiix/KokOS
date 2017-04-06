#pragma once

#include <cpp/string.hpp>

class Module
{
public:
    void init(const char* const strCmd);
    bool compareCommand(const string& strCmd);

private:
    string m_strCmd;
};
