#include <asm.h>

ENTRY(syscall_sysenter)
	push %ecx
	push %edx
	push $SYSENTER_RETURN
	push %ebp
	mov %esp, %ebp
	sysenter
ENTRY(SYSENTER_RETURN)
	pop %ebp
	pop %edx
	pop %edx
	pop %ecx
	ret

/* int write(int fd, char *buffer, int size) */
ENTRY(write)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $4, %eax
	movl 0x8(%ebp), %ebx;	//fd
	movl 0xC(%ebp), %ecx;	//buffer
	movl 0x10(%ebp), %edx;	//size
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -> 
	popl %ebp
	ret

/* Common code for negative return */
nok:
	neg %eax
	mov %eax, errno
	mov $-1, %eax
	popl %ebp
	ret

/* int gettime() */
ENTRY(gettime)
	pushl %ebp
	movl %esp, %ebp
	movl $10, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* int getpid() */
ENTRY(getpid)
	pushl %ebp
	movl %esp, %ebp
	movl $20, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* int gettid() */
ENTRY(gettid)
	pushl %ebp
	movl %esp, %ebp
	movl $21, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* int fork() */
ENTRY(fork)
	pushl %ebp
	movl %esp, %ebp
	movl $2, %eax
	call syscall_sysenter
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret

/* void exit() */
ENTRY(exit)
	pushl %ebp
	movl %esp, %ebp
	movl $1, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* int yield() */
ENTRY(yield)
	pushl %ebp
	movl %esp, %ebp
	movl $13, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* int get_stats(int pid, struct stats *st) */
ENTRY(get_stats)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $35, %eax
	movl 0x8(%ebp), %ebx;	//pid
	movl 0xC(%ebp), %ecx;	//st
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret


/*int getKey(char* b, int timeout);*/
ENTRY(getKey)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $34, %eax
	movl 0x8(%ebp), %ebx;	//char *b
	movl 0xC(%ebp), %ecx;	//int timeout
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret

/* int gotoXY(int x, int y) */
ENTRY(gotoXY)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $25, %eax
	movl 0x8(%ebp), %ebx;	//int x
	movl 0xC(%ebp), %ecx;	//int y
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret

/* int changeColor(int fg, int bg) */
ENTRY(changeColor)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $26, %eax
	movl 0x8(%ebp), %ebx;	//int fg
	movl 0xC(%ebp), %ecx;	//int bg
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret

/* int clrscr(char* b) */
ENTRY(clrscr)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $27, %eax
	movl 0x8(%ebp), %ebx;	//char* b
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret

/**
int threadCreateWithStack( void (*function)(void* arg), int N,
void* parameter )
**/
ENTRY(threadCreateWithStack)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified

	movl $5, %eax
	movl 0x8(%ebp), %ebx;	//function
	movl 0xC(%ebp), %ecx;	//N
	movl 0x10(%ebp), %edx;	//parameter

	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -> 
	popl %ebp
	ret


/* sem_t* semCreate(int initial_value) */
ENTRY(semCreate)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $30, %eax
	movl 0x8(%ebp), %ebx;	//int initial_value
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret

/* int semWait(sem_t* s); */
ENTRY(semWait)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $31, %eax
	movl 0x8(%ebp), %ebx;	//sem_t* s
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret

/* int semSignal(sem_t* s); */
ENTRY(semSignal)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $32, %eax
	movl 0x8(%ebp), %ebx;	//sem_t* s
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret

/* int semDestroy(sem_t* s); */
ENTRY(semDestroy)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $33, %eax
	movl 0x8(%ebp), %ebx;	//sem_t* s
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret

ENTRY(SAVE_REGS)
      pushl %eax
      movl %eax, REGS    //SAVE EAX
      lea REGS, %eax
      movl %ebp, 4(%eax)
      movl %edi, 8(%eax)
      movl %esi, 12(%eax)
      movl %edx, 16(%eax)
      movl %ecx, 20(%eax)
      movl %ebx, 24(%eax)
      popl %eax
      ret

ENTRY(RESTORE_REGS)
      lea REGS  , %eax
      movl 4(%eax) , %ebp 
      movl 8(%eax) , %edi 
      movl 12(%eax), %esi 
      movl 16(%eax), %edx 
      movl 20(%eax), %ecx 
      movl 24(%eax), %ebx 
      movl (%eax), %eax
      ret
