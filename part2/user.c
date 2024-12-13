#include <libc.h>
#include <screen_helper.h>
#include <dangerous_dave.h>

void print_thread(int i){
  char buff[512] = "I'm the new thread";
    write(1, buff, strlen(buff));
    itoa(i, buff);
    write(1, buff, strlen(buff));
    write(1, "\n", 1);

    exit();
  
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
  char buff[512]= "\nI'm the main thread\n";

  int pid = fork();

  int param = 10; // Si es necesario pasar un entero como parámetro
  if(pid == 0) param = 5;
  int t = threadCreateWithStack(print_thread, 1, &param);
  write(1, buff, strlen(buff));
  while(1) { 
    
  }
}