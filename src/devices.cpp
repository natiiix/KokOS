#include "devices.hpp"

#include "terminal.hpp"
#include "memory.hpp"
#include "atapio.hpp"
#include "pci.hpp"

void devices_init(void)
{
    term::writeline("Initializing devices...");

    // -------- IDE --------
    if (probeBus(BUS::PRIMARY))
    {
        if (probeDrive(BUS::PRIMARY, DRIVE::MASTER))
        {
            term::writeline("IDE: Primary Bus : Master Drive");
        }

        if (probeDrive(BUS::PRIMARY, DRIVE::SLAVE))
        {
            term::writeline("IDE: Primary Bus : Slave Drive");
        }
    }
    if (probeBus(BUS::SECONDARY))
    {
        if (probeDrive(BUS::SECONDARY, DRIVE::MASTER))
        {
            term::writeline("IDE: Secondary Bus : Master Drive");
        }

        if (probeDrive(BUS::SECONDARY, DRIVE::SLAVE))
        {
            term::writeline("IDE: Secondary Bus : Slave Drive");
        }
    }

    // -------- PCI --------
    pcidevice* pcidev = nullptr;

	for (uint8_t ibus = 0; ibus < 8; ibus++)
	{
		for (uint8_t islot = 0; islot < 32; islot++)
		{
			pcidev = getDevice(ibus, islot);

            // If a valid device is present
			if (pcidev->vendorid != 0xFFFF && pcidev->deviceid != 0xFFFF)
			{
				term::write("PCI: Bus 0x");
				term::write(ibus, 16);
                term::write(" : Slot 0x");
                term::write(islot, 16);

				if (pcidev->headertype == 0 && pcidev->classid == 1 && pcidev->subclass == 6)
				{
                    term::writeline(" : SATA Mass Storage");
                }
                else
                {
                    term::breakline();
                }
			}

			delete pcidev;
		}
	}
}
