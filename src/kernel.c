#if defined(__linux__)
	#error "You are not using a cross-compiler, you will most certainly run into trouble!"
#endif
 
#if !defined(__i386__)
	#error "This kernel needs to be compiled with a ix86-elf compiler!"
#endif

#include <drivers/memory.h>
#include <io/terminal.h>
#include <drivers/devices.h>
#include <kernel.h>

extern void shell_init(void);

void idt_init(void);
void load_idt_entry(int isr_number, uint32_t base, short int selector, unsigned char flags);
extern void keyboard_handler_int(void);
void kb_init(void);

void kernel_init(void)
{
	mem_init();
	term_init();
	dev_init();

    /*idt_init();
    load_idt_entry(0x21, (uint32_t)keyboard_handler_int, 0x08, 0x8e);
    kb_init();*/
}

void kernel_main(void)
{
	kernel_init(); // Initialize basic components

	//while (true) { }

	shell_init(); // Initialize the shell

	debug_memusage(); // Check for memory leaks
	term_pause(); // Let the user see the memory usage

	kernel_panic_default(); // End of kernel reached, panic
}
