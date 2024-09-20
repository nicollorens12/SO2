#include <asm.h>

ENTRY(addASM)
    push %ebp
    mov %esp, %ebp
    mov 0x8(%esp), %edx
    mov 0xc(%esp), %eax
    add %edx, %eax
    pop %ebp
    ret