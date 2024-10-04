/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

extern int zeos_ticks;

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
  return PID;
}

void sys_exit()
{  
}





#define BUFFER_SIZE 512
char buffer_content[BUFFER_SIZE];

// fd: file descriptor. In this delivery it must always be 1.
// buffer: pointer to the bytes.
// size: number of bytes.
// return ’ Negative number in case of error (specifying the kind of error) and
// 			the number of bytes written if OK.
int sys_write(int fd, char * buffer, int size){

	// CHECK USER PARAMS
	// Check fd (0: no hay error, < 0: id del error)
	int id_error = check_fd(fd, ESCRIPTURA);
	if(id_error < 0) return id_error;

	// Check buffer 
	if(buffer == NULL) return -EFAULT; // TODO - No se que devolver

	// Check size
	if(size < 0) return -EINVAL; // TODO - No se que devolver


	
	int bytes = size;
	int bytes_written; 

	while(bytes > BUFFER_SIZE){
		// Copy data from user
		copy_from_user(buffer, buffer_content, BUFFER_SIZE);
		// Device routine --> write on console
		bytes_written = sys_write_console(buffer_content, BUFFER_SIZE);
		
		buffer = buffer+BUFFER_SIZE; //potser cal sumar bytes_written?
		bytes -= bytes_written;
	}

	// Imprimir los restantes
	copy_from_user(buffer, buffer_content, bytes);
	bytes_written = sys_write_console(buffer_content, bytes);
	bytes -= bytes_written;	


	return size - bytes;
}
