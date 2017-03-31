#include "interrupts.h"
#include "keyboard_map.h"

void kb_init(void)
{
    /* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
    write_port(0x21 , 0xFD);
}

/* Maintain a global location for the current video memory to write to */
static int current_loc = 0;
/* Video memory starts at 0xb8000. Make it a constant pointer to
   characters as this can improve compiler optimization since it
   is a hint that the value of the pointer won't change */
static char *const vidptr = (char*)0xb8000;

void keyboard_handler(void)
{
    unsigned char status;
    signed char keycode;

    /* Acknowledgment */
    status = read_port(0x64);
    /* Lowest bit of status will be set if buffer is not empty */
    if (status & 0x01) {
        keycode = read_port(0x60);
        /* Only print characters on keydown event that have
         * a non-zero mapping */
        if(keycode >= 0 && keyboard_map[keycode]) {
            vidptr[current_loc++] = keyboard_map[keycode];
            /* Attribute 0x07 is white character on black background */
            vidptr[current_loc++] = 0x07;
        }
    }

    /* enable interrupts again */
    write_port(0x20, 0x20);
}
