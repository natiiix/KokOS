#include "pci.hpp"

#include "asm.hpp"
#include "memory.hpp"

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;

    /* create configuration address as per Figure 1 */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
                (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    /* write out the address */
    outl(0xCF8, address);
    /* read in the data */
    /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
    tmp = (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
    return (tmp);
}

uint16_t pciCheckVendor(uint8_t bus, uint8_t slot)
{
    uint16_t vendor, device;
    /* try and read the first configuration register. Since there are no */
    /* vendors that == 0xFFFF, it must be a non-existent device. */
    if ((vendor = pciConfigReadWord(bus,slot,0,0)) != 0xFFFF)
    {
        device = pciConfigReadWord(bus,slot,0,2);
        // ...
    }

    return (vendor);
}

uint16_t pciCheckDevice(uint8_t bus, uint8_t slot)
{
    if (pciConfigReadWord(bus,slot,0,0) != 0xFFFF)
    {
        return pciConfigReadWord(bus,slot,0,2);
    }
    else
    {
        return 0xFFFF;
    }
}

/*void checkDevice(uint8_t bus, uint8_t device)
{
    uint8_t function = 0;

    vendorID = getVendorID(bus, device, function);

    if(vendorID == 0xFFFF) // Device doesn't exist
        return;

    checkFunction(bus, device, function);
    
    headerType = getHeaderType(bus, device, function);
    
    if( (headerType & 0x80) != 0)
    {
        // It is a multi-function device, so check remaining functions
        for(function = 1; function < 8; function++)
        {
            if(getVendorID(bus, device, function) != 0xFFFF)
            {
                checkFunction(bus, device, function);
            }
        }
    }
}*/

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
