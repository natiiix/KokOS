#include <stddef.h>
#include <stdint.h>

#include "memory.hpp"
#include "terminal.hpp"
#include "debug.hpp"
#include "class_vector.hpp"
#include "class_string.hpp"

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
		debug::memusage();
		vector<size_t> vct;
		term::writeline((size_t)vct.getPtr(), 16);
		term::writeline((size_t)vct.getPtrT(), 16);
		debug::memusage();
		vector<vector<size_t>> vct2;
		term::writeline((size_t)vct2.getPtr(), 16);
		term::writeline((size_t)vct2.getPtrT(), 16);
		debug::memusage();
		vct2.push_back(vct);
		debug::memusage();
		vct2.front().resize(200);
		debug::memusage();
		vct2.back().dispose();
		debug::memusage();
		vct2.pop_back();
		debug::memusage();
		vct2.dispose();
		debug::memusage();
		vct.dispose();
		debug::memusage();

		term::pause();
		term::breakline();
	}
}
