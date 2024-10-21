#include <asm.h>

ENTRY(task_switch)
	push %ebp
	mov %esp, %ebp

	# Se guardan los registros (no sabemos si se modificarán luego asi que por seguridad los guardamos)
	push %ebx
	push %esi
	push %edi

	# A inner task switch le pasamos el segundo parámetro (el pcb) que está en 8(%ebp)
	push 8(%ebp)
	call inner_task_switch
	addl $4, %esp

	pop %edi
	pop %esi
	pop %ebx

	mov %ebp,%esp
	pop %ebp
	ret