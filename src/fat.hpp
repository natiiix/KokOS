#pragma once

#include <stddef.h>
#include <stdint.h>

struct FatSector
{
    uint32_t start;
    uint32_t length;
};
