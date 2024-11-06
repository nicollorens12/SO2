# 0 "entry.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "entry.S"




# 1 "include/asm.h" 1
# 6 "entry.S" 2
# 1 "include/segment.h" 1
# 7 "entry.S" 2
# 1 "include/errno.h" 1
# 8 "entry.S" 2
# 71 "entry.S"
.globl keyboard_handler; .type keyboard_handler, @function; .align 0; keyboard_handler:
    pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %ebx; pushl %ecx; pushl %edx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es
    movb $0x20, %al ; outb %al, $0x20 ;
    call keyboard_routine
    popl %edx; popl %ecx; popl %ebx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
    iret

.globl clock_handler; .type clock_handler, @function; .align 0; clock_handler:
      pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %ebx; pushl %ecx; pushl %edx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es
      movb $0x20, %al ; outb %al, $0x20 ;
      call clock_routine
      popl %edx; popl %ecx; popl %ebx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
      iret

.globl system_call_handler; .type system_call_handler, @function; .align 0; system_call_handler:
      pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %ebx; pushl %ecx; pushl %edx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es
      cmpl $0, %EAX
      jl err
      cmpl $MAX_SYSCALL, %EAX
      jg err
      call *sys_call_table(, %EAX, 0x04)
      jmp fin
err:
      movl $-38, %EAX
fin:
      movl %EAX, 0x18(%esp)
      popl %edx; popl %ecx; popl %ebx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
      iret

.globl new_page_fault_handler; .type new_page_fault_handler, @function; .align 0; new_page_fault_handler:
    pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %ebx; pushl %ecx; pushl %edx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es
    # movb $0x20, %al ; outb %al, $0x20 ; no possar (comentat per Juanjo)
    pushl 48(%esp) # Passem parametre (valor de eip --> Adresa erronia)
    call new_page_fault_routine
    # No es necesari fer res perque es quedara en un bucle infinit --> Igualment ho posso
    # ADDL $8, %ESP
    popl %edx; popl %ecx; popl %ebx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
    iret

.globl syscall_handler_sysenter; .type syscall_handler_sysenter, @function; .align 0; syscall_handler_sysenter:
      pushl $0x2B
      pushl %ebp
      pushfl
      pushl $0x23
      pushl 4(%ebp)
      pushl %gs; pushl %fs; pushl %es; pushl %ds; pushl %eax; pushl %ebp; pushl %edi; pushl %esi; pushl %ebx; pushl %ecx; pushl %edx; movl $0x18, %edx; movl %edx, %ds; movl %edx, %es
      cmpl $0, %eax
      jl err_sysenter
      cmpl $MAX_SYSCALL, %eax
      jg err_sysenter
      call *sys_call_table(, %eax, 0x04)
      jmp ok_sysenter
err_sysenter:
      movl $-38, %eax
ok_sysenter:
      movl %eax, 0x18(%esp)
      popl %edx; popl %ecx; popl %ebx; popl %esi; popl %edi; popl %ebp; popl %eax; popl %ds; popl %es; popl %fs; popl %gs;
      movl (%esp), %edx
      movl 12(%esp), %ecx
      sti
      sysexit

.globl writeMSR; .type writeMSR, @function; .align 0; writeMSR:
      push %ebp
      mov %esp, %ebp

      mov 0x8(%ebp), %ecx
      movl $0, %edx
      mov 0xc(%ebp), %eax
      wrmsr

      pop %ebp
      ret
