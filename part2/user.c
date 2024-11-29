#include <libc.h>

char buff[24];

int pid;

enum Colors {
  BLACK = 0b0000,
  BLUE = 0b0001,
  GREEN = 0b0010,
  CYAN = 0b0011,
  RED = 0b0100,
  MAGENTA = 0b0101,
  BROWN = 0b0110,
  LIGHT_GRAY = 0b0111,
  // All color below only work for Foreground [used for Background will raise an error]
  DARK_GRAY = 0b1000,
  LIGHT_BLUE = 0b1001,
  LIGHT_GREEN = 0b1010,
  LIGHT_CYAN = 0b1011,
  LIGHT_RED = 0b1100,
  LIGHT_MAGENTA = 0b1101,
  YELLOW = 0b1110,
  WHITE = 0b1111
}; 

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
  changeColor(YELLOW, RED);

  buff = 'b';
  write(1, &buff, sizeof(buff));

  buff = 'c';
  write(1, &buff, sizeof(buff));

  //char *matrix = NULL;
  char *matrix;
  //clrscr(matrix);

  while(1) { }
}
