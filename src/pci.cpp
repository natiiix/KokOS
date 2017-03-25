#include "pci.hpp"

#include "asm.hpp"
#include "memory.hpp"

pcidevice* getDevice(const uint8_t bus, const uint8_t slot)
{
    uint32_t* devptr = (uint32_t*)mem::alloc(sizeof(pcidevice));

    for (size_t i = 0; i < sizeof(pcidevice); i++)
    {
        uint32_t address = 0x80000000 | ((i << 2) & 0xfc);
        address  |= ((uint32_t)bus) << 16;
        address  |= ((uint32_t)slot) << 11;
        //address  |= ((uint32_t)func) << 8;

        outl(0xCF8, address);
        devptr[i] = inl(0xCFC);
    }

    return (pcidevice*)devptr;
}
