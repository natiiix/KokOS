#pragma once
// http://wiki.osdev.org/PCI

#include <stddef.h>
#include <stdint.h>

struct pcidevice
{
    uint16_t vendorid;
    uint16_t deviceid;

    uint16_t command;
    uint16_t status;

    uint8_t revisionid;
    uint8_t progif;
    uint8_t subclass;
    uint8_t classid;

    uint8_t cachelinesize;
    uint8_t latencytimer;
    uint8_t headertype;
    uint8_t bist;

    uint32_t baseaddr0;
    uint32_t baseaddr1;
    uint32_t baseaddr2;
    uint32_t baseaddr3;
    uint32_t baseaddr4;
    uint32_t baseaddr5;
};

/*uint16_t pciConfigReadWord (uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pciCheckVendor(uint8_t bus, uint8_t slot);
uint16_t pciCheckDevice(uint8_t bus, uint8_t slot);*/
pcidevice* getDevice(const uint8_t bus, const uint8_t slot);
