#include <assembly.h>
#include <drivers/io/terminal.h>

#define IDT_SIZE 256
#define PIC_1_CTRL 0x20
#define PIC_2_CTRL 0xA0
#define PIC_1_DATA 0x21
#define PIC_2_DATA 0xA1

extern void keyboard_handler_int(void);
extern void load_idt(void*);

struct idt_entry
{
    unsigned short int offset_lowerbits;
    unsigned short int selector;
    unsigned char zero;
    unsigned char flags;
    unsigned short int offset_higherbits;
} __attribute__((packed));

struct idt_pointer
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct idt_entry idt_table[IDT_SIZE];
struct idt_pointer idt_ptr;

void load_idt_entry(int isr_number, unsigned long base, short int selector, unsigned char flags)
{
    idt_table[isr_number].offset_lowerbits = base & 0xFFFF;
    idt_table[isr_number].offset_higherbits = (base >> 16) & 0xFFFF;
    idt_table[isr_number].selector = selector;
    idt_table[isr_number].flags = flags;
    idt_table[isr_number].zero = 0;
}

static void initialize_idt_pointer()
{
    idt_ptr.limit = (sizeof(struct idt_entry) * IDT_SIZE) - 1;
    idt_ptr.base = (unsigned int)&idt_table;
}

static void initialize_pic()
{
    /* ICW1 - begin initialization */
    outb(PIC_1_CTRL, 0x11);
    outb(PIC_2_CTRL, 0x11);

    /* ICW2 - remap offset address of idt_table */
    /*
    * In x86 protected mode, we have to remap the PICs beyond 0x20 because
    * Intel have designated the first 32 interrupts as "reserved" for cpu exceptions
    */
    outb(PIC_1_DATA, 0x20);
    outb(PIC_2_DATA, 0x28);

    /* ICW3 - setup cascading */
    outb(PIC_1_DATA, 0x00);
    outb(PIC_2_DATA, 0x00);

    /* ICW4 - environment info */
    outb(PIC_1_DATA, 0x01);
    outb(PIC_2_DATA, 0x01);
    /* Initialization finished */

    /* mask interrupts */
    outb(0x21 , 0xFF);
    outb(0xA1 , 0xFF);
}

void idt_init(void)
{
    initialize_pic();
    initialize_idt_pointer();
    load_idt(&idt_ptr);
}

void interrupts_init(void)
{
    idt_init();
    load_idt_entry(0x21, (unsigned long) keyboard_handler_int, 0x08, 0x8E);
    
    /* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
    outb(0x21 , 0xFD);

    term_writeline("Interrupts initialized.", false);
}
