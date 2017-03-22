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

	term::write("Primary: ");
	putbool(probeBus(BUS::PRIMARY));
	term::write("First: ");
	putbool(probeDrive(BUS::PRIMARY, DRIVE::FIRST));
	term::write("Second: ");
	putbool(probeDrive(BUS::PRIMARY, DRIVE::SECOND));

	term::breakline();

	term::write("Secondary: ");
	putbool(probeBus(BUS::SECONDARY));
	term::write("First: ");
	putbool(probeDrive(BUS::SECONDARY, DRIVE::FIRST));
	term::write("Second: ");
	putbool(probeDrive(BUS::SECONDARY, DRIVE::SECOND));

	while (true)
	{
		term::pause();
	}
}
