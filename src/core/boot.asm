bits 32
section .text
;grub bootloader header
        align 4
        dd 0x1BADB002            ;magic
        dd 0x00                  ;flags
        dd - (0x1BADB002 + 0x00) ;checksum. m+f+c should be zero

global start
extern kernel_main

start:
  mov esp, stack_space  ;set stack pointer
  call kernel_main

; We shouldn't get to here, but just in case do an infinite loop
endloop:
  hlt           ;halt the CPU
  jmp endloop

section .bss
resb 8192       ;8KB for stack
stack_space:
