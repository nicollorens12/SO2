#include <libc.h>

char buff[24];

int pid;

int add(int par1, int par2) {
	return par1 + par2;
}

int addAsm(int, int);

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

	int s = add(0x42, 0x666);

	int s_ass = addAsm(0x42, 0x666);


	// no deuria usar "char *buff" com a string modificable --> TEMPORAL (millor buff[] + strcpy)
	//char buff[] = "\n\n-- Test write --\n\n";

		// copy_data() --> Fer string assign amb aquesta funcio

	// TEST WRITE
	char *buff = "\n\n-- Test write --\n\n";	
	write(1, buff, strlen(buff));

	// TEST GETTIME
	buff = "--Test gettime(): ";
	write(1, buff, strlen(buff));
	itoa(gettime(), buff); 
	write(1, buff, strlen(buff));
	buff = "--\n\n";
	write(1, buff, strlen(buff));
	
    // TEST PAGE FAULT
	//char* p = 0;
	//*p = 'x';

	//Test getpid
	buff = "-- Test getpid() --";
	write(1, buff, strlen(buff));
	int pid = getpid();
	if(pid != -1){
		itoa(pid, buff);
		write(1,buff, strlen(buff));
	}
	else{
		buff = "Error in getpid()";
		write(1, buff, strlen(buff));
		perror();
	}
	
	
	buff = "--\n\n";
	write(1, buff, strlen(buff));


  while(1) { }
}
