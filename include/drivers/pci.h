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

struct pcidevice* getPciDevice(const uint8_t bus, const uint8_t slot, const uint8_t func);
