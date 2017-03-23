#pragma once
// http://www.osdever.net/tutorials/view/lba-hdd-access-via-pio
// http://wiki.osdev.org/ATA_PIO_Mode
// http://wiki.osdev.org/PCI_IDE_Controller

#include <stddef.h>
#include <stdint.h>

enum BUS : uint16_t
{
    PRIMARY = 0x1F0,
    SECONDARY = 0x170,
};

enum DRIVE : uint8_t
{
    MASTER = 0x0,
    SLAVE = 0x1,
};

bool probeBus(const BUS bus);
bool probeDrive(const BUS bus, const DRIVE drive);

char* readLBA28(const BUS bus, const DRIVE drive, const uint32_t addr);
char* readLBA48(const BUS bus, const DRIVE drive, const uint64_t addr);

void writeLBA28(const BUS bus, const DRIVE drive, const uint32_t addr, const char* const buffer);
void writeLBA48(const BUS bus, const DRIVE drive, const uint64_t addr, const char* const buffer);
