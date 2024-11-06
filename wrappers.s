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
 movl %esp, %ebp

 # Save register values (only if modified and are one of: %ebx, %esi, %edi)
 pushl %ebx
 #pushl %esi
 #pushl %edi

 # Move parameters to registers (segons manual EDX, ECX, EBX)
 movl 8(%ebp), %edx
 movl 12(%ebp), %ecx
 movl 16(%ebp), %ebx

 # System call index --> Guardar #servei
 movl $4, %eax

 pushl %ecx
 pushl %edx

 pushl $write_return
 pushl %ebp
 mov %esp,%ebp

 sysenter
write_return:
 popl %ebp
 addl $4, %esp
 popl %edx
 popl %ecx

 cmpl $0, %eax
 jge ok_write
 # en este caso hay error
 negl %eax
 movl %eax, errno
 movl -1, %eax
ok_write:
 popl %ebx
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

.globl getpid; .type getpid, @function; .align 0; getpid:
 push %ebp
 mov %esp,%ebp

 # Save to user stack
 push %ebx
 push %esi
 push %edi

 movl $20, %eax

 push $gp_return
 push %ebp
 mov %esp,%ebp

 sysenter

gp_return:
 pop %ebp
 add $4, %esp
 pop %ebx
 pop %esi
 pop %edi
 cmp $0, %eax
 jge gp_no_error
 neg %eax
 mov %eax, errno
 mov -1, %eax
gp_no_error:
 pop %ebp
 ret


.globl fork; .type fork, @function; .align 0; fork:
 push %ebp
 mov %esp,%ebp

 # Save to user stack
 push %ebx
 push %esi
 push %edi

 movl $2, %eax

 push $f_return
 push %ebp
 mov %esp,%ebp

 sysenter

f_return:
 pop %ebp
 add $4, %esp
 pop %edi
 pop %esi
 pop %ebx
 cmp $0, %eax
 jge f_no_error
 neg %eax
 mov %eax, errno
 mov -1, %eax
f_no_error:
 pop %ebp
 ret
