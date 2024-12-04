#include <asm.h>

/* void switch_stack_thread(void ** save_sp, void * new_sp) Same as switch_stack but takes parameter from stack to %eax */
ENTRY(switch_stack_thread)
    mov 4(%esp), %eax          # La pila al llegar es
    mov %ebp, (%eax)           # | ebp = 0 | <-- esp
    mov 8(%esp), %esp          # |  func   |
	mov %esp, %ebp			   # | &param  |
	subl $4, %ebp
	pushl %ebp
    mov 8(%esp), %edx          
    mov (%edx), %ecx           
	mov %ecx, 8(%esp)		   # Cambiamos de | &param | a | param | es decir ahora no tiene la @ si no el valor
	mov %ecx, 12(%esp)

	popl %ebp
    ret                        # esp apunta a func, por lo que al hacer ret, vamos a ejecutar func param deberia estar a 8(%ebp)



/* void switch_stack(void ** save_sp, void * new_sp) NOT REALLY A FUNCTION!!! */
ENTRY(switch_stack)
	mov 4(%esp), %eax
	mov %ebp, (%eax)
	mov 8(%esp), %esp
	popl %ebp
	ret

/* void task_switch(union task_union *new) */
ENTRY(task_switch)
	pushl %ebp
	movl %esp, %ebp
	pushl %esi
	pushl %edi
	pushl %ebx
	pushl 8(%ebp)
	call inner_task_switch
	addl $4, %esp
	popl %ebx
	popl %edi
	popl %esi
	popl %ebp
	ret

/* void * get_ebp(void) */
ENTRY(get_ebp)
	mov %ebp, %eax
	ret

/* void setMSR(unsigned long msr_number, unsigned long high, unsigned long low)*/
ENTRY(setMSR)
	push %ebp
	movl %esp, %ebp
	movl 0x10(%ebp), %eax
	movl 0xc(%ebp), %edx
	movl 0x8(%esp), %ecx
	wrmsr
	pop %ebp
	ret

