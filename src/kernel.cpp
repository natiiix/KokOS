#include <stddef.h>
#include <stdint.h>

#include "terminal.hpp"
#include "memory.hpp"
#include "debug.hpp"
#include "cstring.hpp"
#include "class_string.hpp"
#include "class_vector.hpp"

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
	term::init();
	mem::init();

	while (true)
	{
		char* cstrinput = term::readline();
		string strinput;
		strinput.push_back(cstrinput);
		delete cstrinput;
		vector<string> vecinput = strinput.split(' ', true);
		
		if (vecinput[0] == "color" && vecinput.size() == 2)
		{
			term::setcolor((uint8_t)cstr::parse(vecinput[1].c_str(), 16));
		}

		for (size_t i = 0; i < vecinput.size(); i++)
		{
			term::write("[", false);
			term::write(cstr::convert(i), true);
			term::write("] ", false);
			term::writeline(vecinput[i], false);
		}

		term::writeline(strinput);

		//delete &strinput;
		strinput.dispose();
		//string::dispose(vecinput);
		vecinput.dispose();

		debug::memusage();
		//debug::panic();
	}
}
