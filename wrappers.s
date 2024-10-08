# 0 "wrappers.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "wrappers.S"
# 1 "include/asm.h" 1
# 2 "wrappers.S" 2

.globl write; .type write, @function; .align 0; write:
 # SAVE THE CONTEXT
 pushl %ebp
 # movl %ebp, %esp
 movl %esp, %ebp

 # Save register values
 pushl %edx
 pushl %ecx
 pushl %ebx

 # Move parameters to registers (segons manual EDX, ECX, EBX)
 movl 8(%ebp), %edx
 movl 12(%ebp), %ecx
 movl 16(%ebp), %ebx

 # System call index --> Guardar #servei
 movl $4, %eax

 pushl $write_return
 pushl %ebp
 mov %esp,%ebp

 sysenter
write_return:
 popl %ebp

 popl %edx
 popl %ecx

 cmpl $0, %eax
 jge ok_write
 # en este caso hay error
 negl %eax
 movl %eax, errno
 movl -1, %eax
ok_write:
 popl %ebp

 ret

.globl gettime; .type gettime, @function; .align 0; gettime:
 pushl %ebp
 movl %esp, %ebp

 # System call index --> Guardar #servei
 movl $10, %eax

 int $0x80

 popl %ebp

 ret
