section .text

global keyboard_handler_int
global load_idt
global load_gdt
extern keyboard_handler

; GDT with a NULL Descriptor, a 32-Bit code Descriptor
; and a 32-bit Data Descriptor
gdt_start:
gdt_null:
    dd 0x0
    dd 0x0

gdt_code:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0

gdt_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0
gdt_end:

; GDT descroptor record
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; Load GDT and set selectors for a flat memory model
load_gdt:
    lgdt [gdt_descriptor]
    jmp CODE_SEG:.setcs              ; Set CS seelctor with far JMP
.setcs:
    mov eax, DATA_SEG                ; Set the Data selectors to defaults
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
    mov ss, eax
    ret

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
