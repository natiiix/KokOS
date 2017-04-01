#if defined(__linux__)
	#error "You are not using a cross-compiler, you will most certainly run into trouble!"
#endif
 
#if !defined(__i386__)
	#error "This kernel needs to be compiled with a ix86-elf compiler!"
#endif

#include <drivers/memory.h>
#include <drivers/io/terminal.h>
#include <drivers/devices.h>
#include <kernel.h>

void shell_init(void);
void interrupts_init(void);

void kernel_init(void)
{
	term_init();
	mem_init();
	dev_init();

	interrupts_init();
}

#include <drivers/io/keyboard.h>

void kernel_main(void)
{
	kernel_init(); // Initialize basic components

	shell_init(); // Initialize the shell

	// ---- UNREACHABLE CODE ----

	debug_memusage(); // Check for memory leaks
	term_pause(); // Let the user see the memory usage

	kernel_panic_default(); // End of kernel reached, panic
}
