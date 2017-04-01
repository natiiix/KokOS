#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <drivers/io/terminal.h>
#include <drivers/memory.h>
#include <drivers/storage/atapio.h>
#include <drivers/pci.h>
#include <drivers/storage/ahci.h>

void dev_init(void)
{
    term_writeline("Initializing devices...", false);

    // -------- IDE --------
    if (probeBus(BUS_PRIMARY))
    {
        if (probeDrive(BUS_PRIMARY, DRIVE_MASTER))
        {
            term_writeline("IDE: Primary Bus : Master Drive", false);
        }

        if (probeDrive(BUS_PRIMARY, DRIVE_SLAVE))
        {
            term_writeline("IDE: Primary Bus : Slave Drive", false);
        }
    }
    if (probeBus(BUS_SECONDARY))
    {
        if (probeDrive(BUS_SECONDARY, DRIVE_MASTER))
        {
            term_writeline("IDE: Secondary Bus : Master Drive", false);
        }

        if (probeDrive(BUS_SECONDARY, DRIVE_SLAVE))
        {
            term_writeline("IDE: Secondary Bus : Slave Drive", false);
        }
    }

    // -------- PCI --------
    struct pcidevice* pcidev;

	for (uint8_t ibus = 0; ibus < 8; ibus++)
	{
		for (uint8_t islot = 0; islot < 32; islot++)
		{
            bool multifunc = false;

            for (uint8_t ifunc = 0; ifunc < 8; ifunc++)
		    {
                pcidev = getPciDevice(ibus, islot, ifunc);

                if (ifunc == 0)
                {
                    multifunc = (pcidev->headertype & 0x80);
                }

                // If a valid device is present
                if (pcidev->vendorid != 0xFFFF && pcidev->deviceid != 0xFFFF)
                {
                    term_write("PCI: Bus 0x", false);
                    term_write_convert(ibus, 16);
                    term_write(" : Slot 0x", false);
                    term_write_convert(islot, 16);
                    term_write(" : Func 0x", false);
                    term_write_convert(ifunc, 16);

                    term_write(" : Hdr ", false);
                    term_write_convert(pcidev->headertype, 16);
                    term_write(" : Cls ", false);
                    term_write_convert(pcidev->classid, 16);
                    term_write(" : Sbcls ", false);
                    term_write_convert(pcidev->subclass, 16);
                    term_write(" : PIF ", false);
                    term_write_convert(pcidev->progif, 16);

                    if (pcidev->headertype == 0 && pcidev->classid == 1 && pcidev->subclass == 6 && pcidev->progif == 1)
                    {
                        //term_writeline(" : SATA Mass Storage");
                        term_writeline(" : SATA", false);
                        HBA_MEM* hbamem = (HBA_MEM*)pcidev->baseaddr5;
                        probe_port(hbamem);

                        /*char* cbuff = (char*)mem_alloc(513);

                        for (size_t i = 0; i < 512; i++)
                        {
                            cbuff[i] = '\0';
                        }

                        if (ahci_read(&hbamem->ports[0], 0, 1, (uint16_t*)cbuff))
                        {
                            for (size_t i = 0; i < 512; i++)
                            {
                                if (cbuff[i] == '\0')
                                    cbuff[i] = '.';
                            }

                            cbuff[512] = '\0';

                            term_writeline(cbuff, false);
                        }
                        else
                        {
                            term_writeline("AHCI drive reading failed!", false);
                        }

                        mem_free(cbuff);*/
                    }
                    else
                    {
                        term_breakline();
                    }
                }

                mem_free(pcidev);

                if (ifunc == 0 && !multifunc)
                {
                    break;
                }
            }
		}
	}

    term_writeline("Devices initialized.", false);
}
