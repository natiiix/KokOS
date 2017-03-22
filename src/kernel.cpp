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

	/*size_t sector = 0;
	string strsec;*/

	while (true)
	{
		term::write("Primary: ");
		putbool(probeController(CONTROLLER::PRIMARY));
		term::write("First: ");
		putbool(probeDrive(CONTROLLER::PRIMARY, DRIVE::FIRST));
		term::write("Second: ");
		putbool(probeDrive(CONTROLLER::PRIMARY, DRIVE::SECOND));

		term::breakline();

		term::write("Secondary: ");
		putbool(probeController(CONTROLLER::SECONDARY));
		term::write("First: ");
		putbool(probeDrive(CONTROLLER::SECONDARY, DRIVE::FIRST));
		term::write("Second: ");
		putbool(probeDrive(CONTROLLER::SECONDARY, DRIVE::SECOND));

		/*char* ptrsec0 = readLBA48(0, 0);
		term::writeline(ptrsec0);
		term::pause();
		writeLBA48(0, 1, ptrsec0);
		delete ptrsec0;

		term::pause();

		char* ptrsec1 = readLBA48(0, 1);
		term::writeline(ptrsec1);
		delete ptrsec1;

		char* ptrsec2 = readLBA48(0, 2);
		term::writeline(ptrsec2);
		delete ptrsec2;

		debug::memusage();*/

		/*char* ptrsec = readLBA48(0, sector++);
		strsec.push_back(ptrsec);
		delete ptrsec;

		if (strsec.contains("SeePlusPlus"))
		{
			term::write("0x");
			term::write(sector);
			term::writeline(":");
			term::writeline(strsec);
			term::pause();
		}

		strsec.clear();*/

		term::pause();
	}
}
