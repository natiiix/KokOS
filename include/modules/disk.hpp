#pragma once

#include <modules/module.hpp>

class Disk : public Module
{
public:
    void process(const string& strArgs);
};
