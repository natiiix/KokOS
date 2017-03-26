#include "devices.hpp"

#include "terminal.hpp"
#include "memory.hpp"
#include "atapio.hpp"
#include "pci.hpp"
#include "ahci.hpp"

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

				if (pcidev->headertype == 0 && pcidev->classid == 1 && pcidev->subclass == 6 && pcidev->progif == 1)
				{
                    term::writeline(" : SATA Mass Storage");

                    HBA_MEM* hbamem = (HBA_MEM*)phystovirt(pcidev->baseaddr5);
                    probe_port(hbamem);
                    //port_rebase(&hbamem->ports[0], 0);
                    
                    char* cbuff = (char*)mem::alloc(513);

                    for (size_t i = 0; i < 512; i++)
                    {
                        cbuff[i] = '\0';
                    }

                    if (read(&hbamem->ports[0], 0, 0, 4096, (uint16_t*)cbuff))
                    {
                        for (size_t i = 0; i < 512; i++)
                        {
                            if (cbuff[i] == '\0')
                                cbuff[i] = '.';
                        }

                        cbuff[512] = '\0';

                        term::writeline(cbuff);
                    }

                    delete cbuff;
                }
                else
                {
                    term::breakline();
                }
			}

			delete pcidev;
		}
	}

    term::writeline("Device initialization completed!");
}
