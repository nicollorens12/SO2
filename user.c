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
    
  while(1) { }
}
