section .multiboot_header
align 8

header_start:
    dd 0xE85250D6
    dd 0
    dd header_end - header_start
    dd 0x100000000 - (0xE85250D6 + 0 + (header_end - header_start))

    ; Framebuffer tag
    align 8
    dw 5
    dw 0
    dd 20

    dd 1024
    dd 768
    dd 32

    ; End tag
    align 8
    dw 0
    dw 0
    dd 8

header_end:

section .text
bits 32

global start
extern kernel_main

start:
    mov esp, stack_top
    and esp, -16
    sub esp, 8

    push ebx
    push eax

    ; cmp eax, 0x36D76289
    ; jne .hang

    call kernel_main

    add esp, 16

.hang:
    cli
    hlt
    jmp .hang

section .bss
align 16

stack_bottom:
    resb 16384

stack_top:
