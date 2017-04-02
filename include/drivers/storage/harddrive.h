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

#define HDD_INVALID 0xFF

uint8_t hddAddIDE(const uint16_t bus, const uint8_t drive);
uint8_t hddAddAHCI(const HBA_PORT* const port);

uint8_t* hddRead(const struct HARDDRIVE hdd, const uint64_t lba);
void hddWrite(const struct HARDDRIVE hdd, const uint64_t lba, const uint8_t* const data);
