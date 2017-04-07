#pragma once

#include <cpp/string.hpp>

class Module
{
public:
    void init(const char* const strCmd);
    bool compare(const string& strCmd);

private:
    string m_strCmd;
};
