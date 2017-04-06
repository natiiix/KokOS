#include <modules/module.hpp>

void Module::init(const char* const strCmd)
{
    m_strCmd.clear();
    m_strCmd.push_back(strCmd);
}

bool Module::compare(const string& strCmd)
{
    return (strCmd == m_strCmd);
}
