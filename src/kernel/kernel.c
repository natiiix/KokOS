#if defined(__linux__)
	#error "You are not using a cross-compiler, you will most certainly run into trouble!"
#endif
 
#if !defined(__i386__)
	#error "This kernel needs to be compiled with a ix86-elf compiler!"
#endif

#include <kernel.h>

void term_init(void);
void mem_init(void);
void dev_init(void);

void interrupts_init(void);
void shell_init(void);

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

	kernel_panic("End of kernel reached!"); // End of kernel reached, panic
}
