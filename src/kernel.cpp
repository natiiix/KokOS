#include "stdtypes.hpp"
#include "memory.hpp"
#include "terminal.hpp"
#include "string.hpp"

// Check if the compiler thinks we are targeting the wrong operating system
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
// This tutorial will only work for the 32-bit ix86 targets
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif
 
#if defined(__cplusplus)
extern "C" // Use C linkage for kernel_main
#endif
void kernel_main(void)
{	
	mem::init();
	term::init();

	while (true)
	{		
		char* cptr = term::readline();
		uint8_t incol = (uint8_t)str::parse(cptr, 16);
		term::setcolor(incol);
		term::memdump(cptr);
		/*mem::free(cptr);
		
		term::write("Base  2: 0b");
		term::memdump(str::convert(inp, 2));
		term::write("Base 10:   ");
		term::memdump(str::convert(inp, 10));
		term::write("Base 16: 0x");
		term::memdump(str::convert(inp, 16));

		term::write("Memory: used 0x");
		term::memdump(str::convert(mem::used(), 16), false);
		term::write(" | empty 0x");
		term::memdump(str::convert(mem::empty(), 16));

		term::breakline();*/
	}
}
