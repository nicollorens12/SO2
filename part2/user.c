#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  
  char *buff;
  

  int pid = fork();

  if(pid == 0){
    char b;
    int resp;
    buff = "Child process\n";
    write(1,buff,strlen(buff));

    resp = getKey(&b, 6);

    buff = "--Test getKey() Child: ";
    write(1,buff,strlen(buff));
    if(resp < 0){
      buff = "Timed out!";
    }
    else buff[0] = b;
    write(1,buff,strlen(buff));
    buff = "\n";
    write(1,buff,strlen(buff));
  }
  else{
    int resp;
    char b;
    buff = "Parent process\n";
    write(1,buff,strlen(buff));

    resp = getKey(&b, 3);

    buff = "--Test getKey() Parent: ";
    write(1,buff,strlen(buff));
    if(resp < 0){
      buff = "Timed out!";
    }
    else buff[0] = b;
    write(1,buff,strlen(buff));
    buff = "\n";
    write(1,buff,strlen(buff));
  }

  
  

  

  while(1) { }
}
