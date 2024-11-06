# 0 "kernel_utils.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "kernel_utils.S"
# 1 "include/asm.h" 1
# 2 "kernel_utils.S" 2

.globl task_switch; .type task_switch, @function; .align 0; task_switch:
 push %ebp
 mov %esp, %ebp

 # Se guardan los registros (no sabemos si se modificarán luego asi que por seguridad los guardamos)
 push %ebx
 push %esi
 push %edi

 push 8(%ebp)
 call inner_task_switch
 addl $4, %esp

 pop %edi
 pop %esi
 pop %ebx

 mov %ebp,%esp
 pop %ebp
 ret

.globl get_ebp; .type get_ebp, @function; .align 0; get_ebp:
 push %ebp
 mov %esp,%ebp

 mov 0(%esp),%eax

 mov %ebp,%esp
 pop %ebp
 ret

.globl set_esp; .type set_esp, @function; .align 0; set_esp:
 push %ebp
 mov %esp,%ebp

 movl 8(%ebp), %esp

 pop %ebp
 ret
