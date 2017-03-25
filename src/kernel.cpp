#include <stddef.h>
#include <stdint.h>

#include "terminal.hpp"
#include "memory.hpp"
#include "debug.hpp"
#include "cstring.hpp"
#include "class_string.hpp"
#include "class_vector.hpp"
#include "atapio.hpp"
#include "pci.hpp"
#include "ahci.hpp"

// Check if the compiler thinks we are targeting the wrong operating system
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
// This tutorial will only work for the 32-bit ix86 targets
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

void putbool(bool b)
{
	term::writeline(b ? "true" : "false");
}

#if defined(__cplusplus)
extern "C" // Use C linkage for kernel_main
#endif
void kernel_main(void)
{
	term::init();
	mem::init();

	/*term::pause();

	term::write("Primary: ");
	putbool(probeBus(BUS::PRIMARY));
	term::write("Master: ");
	putbool(probeDrive(BUS::PRIMARY, DRIVE::MASTER));
	term::write("Slave: ");
	putbool(probeDrive(BUS::PRIMARY, DRIVE::SLAVE));

	term::breakline();

	term::write("Secondary: ");
	putbool(probeBus(BUS::SECONDARY));
	term::write("Master: ");
	putbool(probeDrive(BUS::SECONDARY, DRIVE::MASTER));
	term::write("Slave: ");
	putbool(probeDrive(BUS::SECONDARY, DRIVE::SLAVE));

	term::pause();

	term::writeline(readLBA28(BUS::PRIMARY, DRIVE::MASTER, 0), true);
	term::pause();
	term::writeline(readLBA28(BUS::PRIMARY, DRIVE::SLAVE, 0), true);
	term::pause();
	term::writeline(readLBA28(BUS::SECONDARY, DRIVE::MASTER, 0), true);
	term::pause();
	term::writeline(readLBA28(BUS::SECONDARY, DRIVE::SLAVE, 0), true);
	term::pause();

	for (uint32_t i = 0; i < 32; i++)
	{
		term::writeline(readLBA28(BUS::PRIMARY, DRIVE::MASTER, i), true);
	}*/
		
	/*uint8_t* secMBR = readLBA48(BUS::PRIMARY, DRIVE::MASTER, 0);

	term::writeline((unsigned char)secMBR[510], 16);
	term::writeline((unsigned char)secMBR[511], 16);

	if (secMBR[510] == 0x55 && secMBR[511] == 0xAA)
	{
		term::writeline("MBR: OK");

		char oemName[9];
		oemName[8] = '\0';

		for (size_t i = 0; i < 8; i++)
		{
			oemName[i] = (char)secMBR[3 + i];
		}

		term::writeline(oemName);

		size_t bt = 0x1BE;

		for (size_t i = 0; i < 4; i++)
		{
			for (size_t j = 0; j < 16; j++)
			{
				term::write(secMBR[bt++], 16);
				term::write(" ");
			}

			term::breakline();
		}
	}
	else
	{
		term::writeline("MBR: ERROR");
	}	

	delete secMBR;*/

	pcidevice* pcidev = nullptr;

	for (uint8_t i = 0; i < 8; i++)
	{
		for (uint8_t j = 0; j < 64; j++)
		{
			pcidev = getDevice(i, j);

			if (pcidev->vendorid != 0xFFFF)
			{
				//term::write("[");
				term::write(i, 16);
				term::write(":");
				term::write(j, 16);
				/*term::write("] Ven: ");
				term::write(pcidev->vendorid, 16);
				term::write(" | Dev: ");
				term::write(pcidev->deviceid, 16);*/
				term::write(" | Hea: ");
				term::write(pcidev->headertype, 16);
				term::write(" | Cla: ");
				term::write(pcidev->classid, 16);
				term::write(" | Sub: ");
				term::write(pcidev->subclass, 16);
				term::write(" | BAR5: ");
				term::writeline(pcidev->baseaddr5, 16);

				if (pcidev->headertype == 0 && pcidev->classid == 1 && pcidev->subclass == 6)
				{
					term::write(" SATA");
					//probe_port((HBA_MEM*)pcidev->baseaddr5);
				}
			}

			delete pcidev;
		}
	}

	term::pause();
	debug::panic();

	while (true);
}
