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
void load_gdt(void);

void kernel_main(void)
{
	// Initialize basic components
    load_gdt(); 		// Global Descriptor Table
	term_init(); 		// Terminal
	mem_init(); 		// Memory Management
	dev_init(); 		// Devices
	interrupts_init(); 	// Interrupts

	// Start the Shell module
	shell_init();

	// This should be unreachable code
    static const char PANIC_MESSAGE[] = "End of kernel reached!";
    kernel_panic(PANIC_MESSAGE);
}
