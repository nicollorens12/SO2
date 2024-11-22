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
	movl $-1, %eax
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

ENTRY(getpid)
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
	pop %edi
	pop %esi
	pop %ebx
	cmp $0, %eax
	jge gp_no_error
	neg %eax
	mov %eax, errno
	mov $-1, %eax
gp_no_error:
	pop %ebp
	ret 


ENTRY(fork)
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
	mov $-1, %eax
f_no_error:
	pop %ebp
	ret 


ENTRY(exit)
	push %ebp
	movl %esp,%ebp
	
	# Save to user stack (No seria necessari, ho mantinc per estructura de tots el wrappers)
	push %ebx
	push %esi
	push %edi
	
	movl $1, %eax
	
	push $ex_return
	push %ebp
	movl %esp,%ebp

	sysenter

ex_return:
	pop %ebp
	add $4, %esp
	pop %edi
	pop %esi
	pop %ebx
	cmp $0, %eax
	jge ex_no_error
	neg %eax
	mov %eax, errno
	mov $-1, %eax
ex_no_error:
	pop %ebp
	ret 


ENTRY(block)
	push %ebp
	mov %esp,%ebp
	
	# Save to user stack
	push %ebx
	push %esi
	push %edi
	
	movl $11, %eax
	
	push $gp_return
	push %ebp
	mov %esp,%ebp

	sysenter

bl_return:
	pop %ebp
	add $4, %esp
	pop %edi
	pop %esi
	pop %ebx
	cmp $0, %eax
	jge bl_no_error
	neg %eax
	mov %eax, errno
	mov $-1, %eax
bl_no_error:
	pop %ebp
	ret


ENTRY(unblock)
	push %ebp
	mov %esp,%ebp
	
	# Save to user stack
	push %ebx
	push %esi
	push %edi

	# Move PID parm to register (first parameter)
	movl  8(%ebp), %edx
	
	movl $12, %eax

	push %edx

	push $unbl_return
	push %ebp
	mov %esp,%ebp

	sysenter

unbl_return:
	pop %ebp
	add $4, %esp

	pop %edx
	
	pop %edi
	pop %esi
	pop %ebx

	cmp $0, %eax
	jge unbl_no_error
	neg %eax
	mov %eax, errno
	mov $-1, %eax
unbl_no_error:
	pop %ebp
	ret
