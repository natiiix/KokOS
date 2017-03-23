#include "atapio.hpp"

#include <stddef.h>
#include <stdint.h>

#include "asm.hpp"
#include "memory.hpp"

// ATA port I/O register offsets
const uint16_t REGISTER_DATA         = 0x0;
const uint16_t REGISTER_ERROR        = 0x1;
const uint16_t REGISTER_SECTOR_COUNT = 0x2;
const uint16_t REGISTER_LBA_LOW      = 0x3;
const uint16_t REGISTER_LBA_MID      = 0x4;
const uint16_t REGISTER_LBA_HIGH     = 0x5;
const uint16_t REGISTER_DRIVE_HEAD   = 0x6;
const uint16_t REGISTER_COMMAND      = 0x7;
const uint16_t REGISTER_STATUS       = 0x7;
//const uint16_t REGISTER_ALTERNATE_STATUS = 0x106;

const uint8_t COMMAND_READ           = 0x20;
const uint8_t COMMAND_READ_EXTENDED  = 0x24;
const uint8_t COMMAND_WRITE          = 0x30;
const uint8_t COMMAND_WRITE_EXTENDED = 0x34;

const uint8_t STATUS_BSY  = 0x80; // Busy
const uint8_t STATUS_DRDY = 0x40; // Drive ready
const uint8_t STATUS_DWF  = 0x20; // Drive write fault
const uint8_t STATUS_DSC  = 0x10; // Drive seek complete
const uint8_t STATUS_DRQ  = 0x08; // Data request ready
const uint8_t STATUS_CORR = 0x04; // Corrected data
const uint8_t STATUS_IDX  = 0x02; // Index
const uint8_t STATUS_ERR  = 0x01; // Error

const uint8_t PROBE_BYTE = 0xAB;

//const uint8_t PROBE_DRIVE_MASTER = 0xA0;
//const uint8_t PROBE_DRIVE_SLAVE = 0xB0;

void awaitStatus(const BUS bus, const uint8_t status)
{
    while (!(inb(bus + REGISTER_STATUS) & status)) { }
}

void awaitStatusFalse(const BUS bus, const uint8_t status)
{
    while (inb(bus + REGISTER_STATUS) & status) { }
}

bool probeBus(const BUS bus)
{
    outb(bus + REGISTER_LBA_LOW, PROBE_BYTE);
    uint8_t probeResponse = inb(bus + REGISTER_LBA_LOW);

    return (probeResponse == PROBE_BYTE);
}

bool probeDrive(const BUS bus, const DRIVE drive)
{
    // Convert drive from 0/1 to 0xA0/0xB0
    uint8_t probeDrive = 0xA0 + (drive << 4);

    outb(bus + REGISTER_DRIVE_HEAD, probeDrive);
    uint8_t probeResponse = inb(bus + REGISTER_STATUS);

    return (probeResponse & STATUS_DRDY);
}

void setupLBA28(const BUS bus, const DRIVE drive, const uint32_t addr)
{
    outb(bus + REGISTER_ERROR, 0x00);
    outb(bus + REGISTER_SECTOR_COUNT, 0x01);

    outb(bus + REGISTER_LBA_LOW, (uint8_t)addr);
    outb(bus + REGISTER_LBA_MID, (uint8_t)(addr >> 8));
    outb(bus + REGISTER_LBA_HIGH, (uint8_t)(addr >> 16));

    outb(bus + REGISTER_DRIVE_HEAD, 0xE0 | (drive << 4) | ((addr >> 24) & 0x0F));
}

void setupLBA48(const BUS bus, const DRIVE drive, const uint64_t addr)
{
    outb(bus + REGISTER_ERROR, 0x00);
    outb(bus + REGISTER_ERROR, 0x00);

    outb(bus + REGISTER_SECTOR_COUNT, 0x00);
    outb(bus + REGISTER_SECTOR_COUNT, 0x01);

    outb(bus + REGISTER_LBA_LOW, (uint8_t)(addr >> 24));
    outb(bus + REGISTER_LBA_LOW, (uint8_t)addr);
    outb(bus + REGISTER_LBA_MID, (uint8_t)(addr >> 32));
    outb(bus + REGISTER_LBA_MID, (uint8_t)(addr >> 8));
    outb(bus + REGISTER_LBA_HIGH, (uint8_t)(addr >> 40));
    outb(bus + REGISTER_LBA_HIGH, (uint8_t)(addr >> 16));
    
    outb(bus + REGISTER_DRIVE_HEAD, 0x40 | (drive << 4));
}

char* readLBA(const BUS bus)
{
    char* ptrBuff = (char*)mem::alloc(513);
    ptrBuff[512] = '\0';
    uint16_t tmpword = 0;

    awaitStatus(bus, STATUS_DRQ);

    for (size_t idx = 0; idx < 256; idx++)
    {
        tmpword = inw(bus + REGISTER_DATA);
        ptrBuff[(idx << 1)] = (uint8_t)tmpword;
        ptrBuff[(idx << 1) + 1] = (uint8_t)(tmpword >> 8);
    }

    for (size_t i = 0; i < 512; i++)
    {
        if (ptrBuff[i] == '\0')
        {
            ptrBuff[i] = '.';
        }
    }

    return ptrBuff;
}

char* readLBA28(const BUS bus, const DRIVE drive, const uint32_t addr)
{
    setupLBA28(bus, drive, addr);
    outb(bus + REGISTER_COMMAND, COMMAND_READ);
    return readLBA(bus);
}

char* readLBA48(const BUS bus, const DRIVE drive, const uint64_t addr)
{
    setupLBA48(bus, drive, addr);
    outb(bus + REGISTER_COMMAND, COMMAND_READ_EXTENDED);
    return readLBA(bus);
}

void writeLBA(const BUS bus, const char* const buffer)
{
    awaitStatus(bus, STATUS_DRQ);

    for (size_t idx = 0; idx < 256; idx++)
    {
        outw(bus + REGISTER_DATA, ((uint16_t)buffer[idx << 1]) | (((uint16_t)buffer[(idx << 1) + 1]) << 8));
    }
}

void writeLBA28(const BUS bus, const DRIVE drive, const uint32_t addr, const char* const buffer)
{
    setupLBA28(bus, drive, addr);
    outb(bus + REGISTER_COMMAND, COMMAND_WRITE);
    writeLBA(bus, buffer);
}

void writeLBA48(const BUS bus, const DRIVE drive, const uint64_t addr, const char* const buffer)
{
    setupLBA48(bus, drive, addr);
    outb(bus + REGISTER_COMMAND, COMMAND_WRITE_EXTENDED);
    writeLBA(bus, buffer);
}
