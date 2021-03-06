#pragma once

#include <stddef.h>
#include <stdint.h>
#include <drivers/storage/ahci.h>

#define HDD_TYPE_UNKNOWN 0
#define HDD_TYPE_IDE 1
#define HDD_TYPE_AHCI 2

struct HARDDRIVE
{
    uint8_t type;
    size_t addr;
    // IDE: addr = (BUS << 8) + DRIVE
    // AHCI: addr = HBA_PORT*
};

extern struct HARDDRIVE hddArray[0x10];
extern uint8_t hddCount;

#if defined(__cplusplus)
extern "C"
#endif
char* getHddInfoStr(const uint8_t hddIdx);

void hddAddIDE(const uint16_t bus, const uint8_t drive);
void hddAddAHCI(const HBA_PORT* const port);

uint8_t* hddRead(const uint8_t hddIdx, const uint64_t lba);
void hddWrite(const uint8_t hddIdx, const uint64_t lba, const uint8_t* const data);
