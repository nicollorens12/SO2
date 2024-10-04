#include <asm.h>

ENTRY(write)
	## SAVE THE CONTEXT
	pushl %ebp
	#movl %ebp, %esp
	movl %esp, %ebp

	# Save register values
	pushl %edx
	pushl %ecx
	pushl %ebx

	# Move parameters to registers (segons manual EDX, ECX, EBX)
	movl  8(%ebp), %edx		
	movl 12(%ebp), %ecx
	movl 16(%ebp), %ebx

	# System call index --> Guardar #servei
	movl $4, %eax

	int $0x80

	cmpl $0, %EAX
    jg ok
err:
	movl %eax, errno
	movl $-1, %eax
ok:
	popl %ebx
	popl %ecx
	popl %edx

	popl %ebp

	ret