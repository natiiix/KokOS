#pragma once
// http://www.osdever.net/tutorials/view/lba-hdd-access-via-pio
// http://wiki.osdev.org/ATA_PIO_Mode
// http://wiki.osdev.org/PCI_IDE_Controller

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

enum BUS
{
    BUS_PRIMARY = 0x1F0,
    BUS_SECONDARY = 0x170,
};

enum DRIVE
{
    DRIVE_MASTER = 0x0,
    DRIVE_SLAVE = 0x1,
};

bool probeBus(const enum BUS bus);
bool probeDrive(const enum BUS bus, const enum DRIVE drive);

uint8_t* readLBA28(const enum BUS bus, const enum DRIVE drive, const uint32_t addr);
uint8_t* readLBA48(const enum BUS bus, const enum DRIVE drive, const uint64_t addr);

void writeLBA28(const enum BUS bus, const enum DRIVE drive, const uint32_t addr, const uint8_t* const buffer);
void writeLBA48(const enum BUS bus, const enum DRIVE drive, const uint64_t addr, const uint8_t* const buffer);
