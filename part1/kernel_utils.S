#include <asm.h>

ENTRY(task_switch)
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

ENTRY(get_ebp)
	push %ebp
	mov %esp,%ebp

	mov 0(%esp),%eax

	mov %ebp,%esp
	pop %ebp
	ret

ENTRY(set_esp)
	push %ebp
	mov %esp,%ebp

	movl 8(%ebp), %esp

	pop %ebp
	ret