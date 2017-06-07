#include <drivers/storage/atapio.h>

#include <stddef.h>
#include <stdint.h>

#include <assembly.h>
#include <drivers/memory.h>

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
const uint8_t COMMAND_IDENTIFY       = 0xEC;

const uint8_t STATUS_BUSY           = 0x80;
const uint8_t STATUS_READY          = 0x40;
const uint8_t STATUS_WRITE_FAULT    = 0x20;
const uint8_t STATUS_SEEK_COMPLETE  = 0x10;
const uint8_t STATUS_REQUEST_READY  = 0x08;
const uint8_t STATUS_CORRECTED_DATA = 0x04;
const uint8_t STATUS_INDEX          = 0x02;
const uint8_t STATUS_ERROR          = 0x01;

const uint8_t PROBE_BYTE = 0xAB;

//const uint8_t PROBE_DRIVE_MASTER = 0xA0;
//const uint8_t PROBE_DRIVE_SLAVE = 0xB0;

void awaitStatus(const enum BUS bus, const uint8_t status)
{
    while (!(inb(bus + REGISTER_STATUS) & status)) { }
}

void awaitStatusFalse(const enum BUS bus, const uint8_t status)
{
    while (inb(bus + REGISTER_STATUS) & status) { }
}

void switchDrive(const enum BUS bus, const enum DRIVE drive)
{
    // Convert drive from 0/1 to 0xA0/0xB0
    uint8_t driveValue = 0xA0 + (drive << 4);
    outb(bus + REGISTER_DRIVE_HEAD, driveValue);

    // The notorious ATAPIO 400ns wait
    inb(bus + REGISTER_STATUS);
    inb(bus + REGISTER_STATUS);
    inb(bus + REGISTER_STATUS);
    inb(bus + REGISTER_STATUS);
}

bool ideIdentify(const enum BUS bus, const enum DRIVE drive)
{
    switchDrive(bus, drive);

    outb(bus + REGISTER_SECTOR_COUNT, 0);
    outb(bus + REGISTER_LBA_LOW, 0);
    outb(bus + REGISTER_LBA_MID, 0);
    outb(bus + REGISTER_LBA_HIGH, 0);

    outb(bus + REGISTER_COMMAND, COMMAND_IDENTIFY);
    uint8_t status = inb(bus + REGISTER_STATUS);

    if (status)
    {
        awaitStatusFalse(bus, STATUS_BUSY);

        if (!inb(bus + REGISTER_LBA_MID) && !inb(bus + REGISTER_LBA_HIGH))
        {
            awaitStatus(bus, STATUS_REQUEST_READY | STATUS_ERROR);

            if (inb(bus + REGISTER_STATUS) & STATUS_REQUEST_READY)
            {
                // Read the indentification data from the drive
                for (size_t idx = 0; idx < 256; idx++)
                {
                    // The identification data MUST BE read from the drive
                    // so let's read it and then dump it immediately
                    // because we don't need it for anything
                    inw(bus + REGISTER_DATA);
                }

                return true;
            }
        }
    }

    return false;
}

bool probeBus(const enum BUS bus)
{
    outb(bus + REGISTER_LBA_LOW, PROBE_BYTE);
    uint8_t probeResponse = inb(bus + REGISTER_LBA_LOW);

    return (probeResponse == PROBE_BYTE);
}

bool probeDrive(const enum BUS bus, const enum DRIVE drive)
{
    // ideIdentify() is used to tell ATA apart from ATAPI which we can't read from
    bool identified = ideIdentify(bus, drive);

    switchDrive(bus, drive);
    uint8_t status = inb(bus + REGISTER_STATUS);

    // Only returns true if the drive is an ATA storage device
    return (status & STATUS_READY) && identified;
}

void setupLBA28(const enum BUS bus, const enum DRIVE drive, const uint32_t addr)
{
    outb(bus + REGISTER_ERROR, 0x00);
    outb(bus + REGISTER_SECTOR_COUNT, 0x01);

    outb(bus + REGISTER_LBA_LOW, (uint8_t)addr);
    outb(bus + REGISTER_LBA_MID, (uint8_t)(addr >> 8));
    outb(bus + REGISTER_LBA_HIGH, (uint8_t)(addr >> 16));

    outb(bus + REGISTER_DRIVE_HEAD, 0xE0 | (drive << 4) | ((addr >> 24) & 0x0F));
}

void setupLBA48(const enum BUS bus, const enum DRIVE drive, const uint64_t addr)
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

uint8_t* readLBA(const enum BUS bus)
{
    uint8_t* ptrBuff = (uint8_t*)mem_dynalloc(512);
    uint16_t tmpword = 0;

    awaitStatus(bus, STATUS_REQUEST_READY);

    for (size_t idx = 0; idx < 256; idx++)
    {
        tmpword = inw(bus + REGISTER_DATA);
        ptrBuff[(idx << 1)] = (uint8_t)tmpword;
        ptrBuff[(idx << 1) + 1] = (uint8_t)(tmpword >> 8);
    }

    return ptrBuff;
}

uint8_t* readLBA28(const enum BUS bus, const enum DRIVE drive, const uint32_t addr)
{
    setupLBA28(bus, drive, addr);
    outb(bus + REGISTER_COMMAND, COMMAND_READ);
    return readLBA(bus);
}

uint8_t* readLBA48(const enum BUS bus, const enum DRIVE drive, const uint64_t addr)
{
    setupLBA48(bus, drive, addr);
    outb(bus + REGISTER_COMMAND, COMMAND_READ_EXTENDED);
    return readLBA(bus);
}

void writeLBA(const enum BUS bus, const uint8_t* const buffer)
{
    awaitStatus(bus, STATUS_REQUEST_READY);

    for (size_t idx = 0; idx < 256; idx++)
    {
        outw(bus + REGISTER_DATA, ((uint16_t)buffer[idx << 1]) | (((uint16_t)buffer[(idx << 1) + 1]) << 8));
    }

    awaitStatusFalse(bus, STATUS_BUSY);
}

void writeLBA28(const enum BUS bus, const enum DRIVE drive, const uint32_t addr, const uint8_t* const buffer)
{
    setupLBA28(bus, drive, addr);
    outb(bus + REGISTER_COMMAND, COMMAND_WRITE);
    writeLBA(bus, buffer);
}

void writeLBA48(const enum BUS bus, const enum DRIVE drive, const uint64_t addr, const uint8_t* const buffer)
{
    setupLBA48(bus, drive, addr);
    outb(bus + REGISTER_COMMAND, COMMAND_WRITE_EXTENDED);
    writeLBA(bus, buffer);
}
