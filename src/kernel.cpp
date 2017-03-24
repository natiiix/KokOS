#include <stddef.h>
#include <stdint.h>

#include "terminal.hpp"
#include "memory.hpp"
#include "debug.hpp"
#include "cstring.hpp"
#include "class_string.hpp"
#include "class_vector.hpp"
#include "atapio.hpp"

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
		
	uint8_t* secMBR = readLBA48(BUS::PRIMARY, DRIVE::MASTER, 0);

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

	delete secMBR;

	term::pause();
	debug::panic();

	while (true);
}
