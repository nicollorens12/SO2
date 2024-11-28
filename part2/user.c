#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  // int resp;
  // char *b;
  // resp = getKey(b, 10);
  // char *buff;

  // buff = "--Test getKey(): ";
  // write(1,buff,sizeof(buff));
  // if(resp < 0){
  //   buff = "Timed out!";
  // }
  // else buff = b;
  // write(1,buff,sizeof(buff));
  // buff = "\n";
  // write(1,buff,sizeof(buff));



  // ------------------------
  // PRUEBAS PARA PANTALLA
  // ------------------------
  //char buff[] = "--Test getKey(): ";
  char buff = 'c';
  write(1, &buff, sizeof(buff));

  buff = 'b';
  write(1, &buff, sizeof(buff));


  gotoXY(4,4);
  changeColor(0b001, 0b0010);

  buff = 'b';
  write(1, &buff, sizeof(buff));

  buff = 'c';
  write(1, &buff, sizeof(buff));

  //char *matrix = NULL;
  char *matrix;
  clrscr(matrix);

  while(1) { }
}
