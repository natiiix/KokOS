#include "fat.hpp"

FatSector secJump;
FatSector secOEMName;

void init(void)
{
    secJump.start = 0x0;
    secJump.length = 0x3;

    secOEMName.start = 0x3;
    secOEMName.length = 0x8;
}
