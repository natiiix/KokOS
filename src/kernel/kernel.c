#if defined(__linux__)
	#error "You are not using a cross-compiler, you will most certainly run into trouble!"
#endif
 
#if !defined(__i386__)
	#error "This kernel needs to be compiled with a ix86-elf compiler!"
#endif

#include <kernel.h>

// These _init() functions are not in their respective headers because
// they're supposed to be never called from anywhere else than from here

void term_init(void);
void mem_init(void);
void dev_init(void);

void interrupts_init(void);
void shell_init(void);

void kernel_main(void)
{
	// Initialize basic components
	term_init();
	mem_init();
	dev_init();
	interrupts_init();

	// Start the Shell module
	shell_init();

	// This should be unreachable code
	kernel_panic("End of kernel reached!");
}
