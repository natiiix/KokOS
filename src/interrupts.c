#include <stddef.h>
#include <stdint.h>
#include <assembly.h>
#include <io/terminal.h>

unsigned char keyboard_map[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',     /* 9 */
  '9', '0', '-', '=', '\b',     /* Backspace */
  '\t',                 /* Tab */
  'q', 'w', 'e', 'r',   /* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
    0,                  /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',     /* 39 */
 '\'', '`',   0,                /* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',                    /* 49 */
  'm', ',', '.', '/',   0,                              /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

void kb_init(void)
{
    /* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
    outb(0x21 , 0xFD);
}

/* Maintain a global location for the current video memory to write to */
//static int current_loc = 0;
/* Video memory starts at 0xb8000. Make it a constant pointer to
   characters as this can improve compiler optimization since it
   is a hint that the value of the pointer won't change */
//static char *const vidptr = (char*)0xb8000;

void keyboard_handler(void)
{
    unsigned char status;
    signed char keycode;

    /* Acknowledgment */
    status = inb(0x64);
    /* Lowest bit of status will be set if buffer is not empty */
    if (status & 0x01) {
        keycode = inb(0x60);
        /* Only print characters on keydown event that have
         * a non-zero mapping */
        if(keycode >= 0 && keyboard_map[keycode])
        {
            term_writeline_convert(keyboard_map[keycode], 10);
            //vidptr[current_loc++] = keyboard_map[keycode];
            /* Attribute 0x07 is white character on black background */
            //vidptr[current_loc++] = 0x07;
        }
    }

    /* enable interrupts again */
    outb(0x20, 0x20);
}

#define IDT_SIZE 256
#define PIC_1_CTRL 0x20
#define PIC_2_CTRL 0xA0
#define PIC_1_DATA 0x21
#define PIC_2_DATA 0xA1

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

void load_idt_entry(int isr_number, uint32_t base, short int selector, unsigned char flags)
{
    idt_table[isr_number].offset_lowerbits = (uint16_t)base;
    idt_table[isr_number].offset_higherbits = (uint16_t)(base >> 16);
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
    outb(0x21 , 0xff);
    outb(0xA1 , 0xff);
}

void idt_init()
{
    initialize_pic();
    initialize_idt_pointer();
    load_idt(&idt_ptr);
}
