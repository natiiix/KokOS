section .text

extern keyboard_handler
global read_port
global write_port
global load_idt
global keyboard_handler_int

keyboard_handler_int:
    pushad
    cld
    call keyboard_handler
    popad
    iretd

load_idt:
    mov edx, [esp + 4]
    lidt [edx]
    sti
    ret
