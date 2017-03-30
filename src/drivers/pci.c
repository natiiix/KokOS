#include <drivers/pci.h>
#include <assembly.h>
#include <drivers/memory.h>

struct pcidevice* getPciDevice(const uint8_t bus, const uint8_t slot, const uint8_t func)
{
    uint32_t* devptr = (uint32_t*)mem_alloc(sizeof(struct pcidevice));

    for (size_t i = 0; i < sizeof(struct pcidevice); i++)
    {
        uint32_t address = 0x80000000 | ((i << 2) & 0xfc);
        address  |= ((uint32_t)bus) << 16;
        address  |= ((uint32_t)slot) << 11;
        address  |= ((uint32_t)func) << 8;

        outl(0xCF8, address);
        devptr[i] = inl(0xCFC);
    }

    return (struct pcidevice*)devptr;
}
