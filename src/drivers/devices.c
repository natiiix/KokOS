#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <drivers/io/terminal.h>
#include <drivers/memory.h>
#include <drivers/storage/atapio.h>
//#include <drivers/pci.h>
//#include <drivers/storage/ahci.h>
#include <drivers/storage/harddrive.h>

void dev_init(void)
{
    term_writeline("Initializing devices...", false);

    // -------- IDE --------
    if (probeBus(BUS_PRIMARY))
    {
        if (probeDrive(BUS_PRIMARY, DRIVE_MASTER))
        {
            term_writeline("IDE: Primary Bus : Master Drive", false);
            hddAddIDE(BUS_PRIMARY, DRIVE_MASTER);
        }

        if (probeDrive(BUS_PRIMARY, DRIVE_SLAVE))
        {
            term_writeline("IDE: Primary Bus : Slave Drive", false);
            hddAddIDE(BUS_PRIMARY, DRIVE_SLAVE);
        }
    }
    if (probeBus(BUS_SECONDARY))
    {
        if (probeDrive(BUS_SECONDARY, DRIVE_MASTER))
        {
            term_writeline("IDE: Secondary Bus : Master Drive", false);
            hddAddIDE(BUS_SECONDARY, DRIVE_MASTER);
        }

        if (probeDrive(BUS_SECONDARY, DRIVE_SLAVE))
        {
            term_writeline("IDE: Secondary Bus : Slave Drive", false);
            hddAddIDE(BUS_SECONDARY, DRIVE_SLAVE);
        }
    }

    // -------- PCI --------
    /*struct pcidevice* pcidev;

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
                    //term_write("PCI: Bus ", false);
                    //term_write_convert(ibus, 10);
                    //term_write(" : Slot ", false);
                    //term_write_convert(islot, 10);
                    //term_write(" : Func ", false);
                    //term_write_convert(ifunc, 10);

                    //term_write(" : 0x", false); // Header type
                    //term_write_convert(pcidev->headertype, 16);
                    //term_write(" : 0x", false); // Class
                    //term_write_convert(pcidev->classid, 16);
                    //term_write(" : 0x", false); // Subclass
                    //term_write_convert(pcidev->subclass, 16);
                    //term_write(" : 0x", false); // Prog IF
                    //term_write_convert(pcidev->progif, 16);

                    if (pcidev->headertype == 0 && pcidev->classid == 1 && pcidev->subclass == 6 && pcidev->progif == 1)
                    {
                        term_write("PCI: Bus ", false);
                        term_write_convert(ibus, 10);
                        term_write(" : Slot ", false);
                        term_write_convert(islot, 10);
                        term_write(" : Func ", false);
                        term_write_convert(ifunc, 10);

                        term_writeline(" : SATA Mass Storage", false);
                        HBA_MEM* hbamem = (HBA_MEM*)pcidev->baseaddr5;
                        probe_port(hbamem);
                    }
                    else
                    {
                        //term_breakline();
                    }
                }

                mem_free(pcidev);

                if (ifunc == 0 && !multifunc)
                {
                    break;
                }
            }
		}
	}*/

    term_writeline_convert(hddCount, 10);
    term_writeline("Devices initialized.", false);
}
