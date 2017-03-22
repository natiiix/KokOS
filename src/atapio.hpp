#pragma once
// http://www.osdever.net/tutorials/view/lba-hdd-access-via-pio

#include <stddef.h>
#include <stdint.h>

enum CONTROLLER : uint16_t
{
    PRIMARY = 0x1F0,
    SECONDARY = 0x170,
};

enum DRIVE : uint16_t
{
    FIRST = 0xA0,
    SECOND = 0xB0,
};

bool probeController(const CONTROLLER controller);
bool probeDrive(const CONTROLLER controller, const DRIVE drive);

char* readLBA28(const uint8_t drive, const uint32_t addr);
char* readLBA48(const uint8_t drive, const uint64_t addr);

void writeLBA28(const uint8_t drive, const uint32_t addr, const char* const buffer);
void writeLBA48(const uint8_t drive, const uint64_t addr, const char* const buffer);
