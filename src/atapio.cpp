#include "atapio.hpp"
// http://www.osdever.net/tutorials/view/lba-hdd-access-via-pio

#include <stddef.h>
#include <stdint.h>

#include "asm.hpp"
#include "memory.hpp"

bool probeController(const CONTROLLER controller)
{
    const uint8_t probeByte = 0xAB;

    outb(controller + 3, probeByte);
    uint8_t probeResponse = inb(controller + 3);

    return (probeResponse == probeByte);
}

bool probeDrive(const CONTROLLER controller, const DRIVE drive)
{
    outb(controller + 6, drive);
    uint8_t probeResponse = inb(controller + 7);

    return (probeResponse & 0x40);
}

void setupLBA28(const uint8_t drive, const uint32_t addr)
{
    outb(0x1F1, 0x00);
    outb(0x1F2, 0x01);

    outb(0x1F3, (uint8_t)addr);
    outb(0x1F4, (uint8_t)(addr >> 8));
    outb(0x1F5, (uint8_t)(addr >> 16));

    outb(0x1F6, 0xE0 | (drive << 4) | ((addr >> 24) & 0x0F));
}

void setupLBA48(const uint8_t drive, const uint64_t addr)
{
    outb(0x1F1, 0x00);
    outb(0x1F1, 0x00);

    outb(0x1F2, 0x00);
    outb(0x1F2, 0x01);

    outb(0x1F3, (uint8_t)(addr >> 24));
    outb(0x1F3, (uint8_t)addr);
    outb(0x1F4, (uint8_t)(addr >> 32));
    outb(0x1F4, (uint8_t)(addr >> 8));
    outb(0x1F5, (uint8_t)(addr >> 40));
    outb(0x1F5, (uint8_t)(addr >> 16));
    
    outb(0x1F6, 0x40 | (drive << 4));
}

char* readLBA(void)
{
    while (!(inb(0x1F7) & 0x08));

    char* ptrBuff = (char*)mem::alloc(513);
    ptrBuff[512] = '\0';
    uint16_t tmpword = 0;
    
    for (size_t idx = 0; idx < 256; idx++)
    {
        tmpword = inw(0x1F0);
        ptrBuff[(idx << 2)] = (uint8_t)tmpword;
        ptrBuff[(idx << 2) + 1] = (uint8_t)(tmpword >> 8);
    }

    for (size_t i = 0; i < 512; i++)
    {
        if (ptrBuff[i] == '\0')
        {
            ptrBuff[i] = ' ';
        }
    }

    return ptrBuff;
}

char* readLBA28(const uint8_t drive, const uint32_t addr)
{
    setupLBA28(drive, addr);
    outb(0x1F7, 0x20);
    return readLBA();
}

char* readLBA48(const uint8_t drive, const uint64_t addr)
{
    setupLBA48(drive, addr);
    outb(0x1F7, 0x24);
    return readLBA();
}

void writeLBA(const char* const buffer)
{
    while (!(inb(0x1F7) & 0x08));

    for (size_t idx = 0; idx < 256; idx++)
    {
        outw(0x1F0, ((uint16_t)buffer[idx << 1]) | (((uint16_t)buffer[(idx << 1) + 1]) << 8));
    }
}

void writeLBA28(const uint8_t drive, const uint32_t addr, const char* const buffer)
{
    setupLBA28(drive, addr);
    outb(0x1F7, 0x30);
    writeLBA(buffer);
}

void writeLBA48(const uint8_t drive, const uint64_t addr, const char* const buffer)
{
    setupLBA48(drive, addr);
    outb(0x1F7, 0x34);
    writeLBA(buffer);
}
