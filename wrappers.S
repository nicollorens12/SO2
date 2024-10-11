#include <asm.h>

ENTRY(write)
	# SAVE THE CONTEXT
	pushl %ebp
	movl %esp, %ebp

	# Save register values (only if modified and are one of: %ebx, %esi, %edi)
	pushl %ebx
	#pushl %esi
	#pushl %edi	

	# Move parameters to registers (segons manual EDX, ECX, EBX)
	movl  8(%ebp), %edx		
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

ENTRY(gettime)
	pushl %ebp
	movl %esp, %ebp

	# System call index --> Guardar #servei
	movl $10, %eax

	int $0x80

	popl %ebp

	ret