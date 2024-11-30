#include <libc.h>
#include <screen_helper.h>
#include <dangerous_dave.h>

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
  draw_map();
  draw_player();

  while(1) { }
}

//if(pid == 0){
//    char b;
//    int resp;
//    buff = "Child process\n";
//    write(1,buff,strlen(buff));
//
//    resp = getKey(&b, 6);
//
//    buff = "--Test getKey() Child: ";
//    write(1,buff,strlen(buff));
//    if(resp < 0){
//      buff = "Timed out!";
//    }
//    else buff[0] = b;
//    write(1,buff,strlen(buff));
//    buff = "\n";
//    write(1,buff,strlen(buff));
//  }
//  else{
//    int resp;
//    char b;
//    buff = "Parent process\n";
//    write(1,buff,strlen(buff));
//
//    resp = getKey(&b, 3);
//
//    buff = "--Test getKey() Parent: ";
//    write(1,buff,strlen(buff));
//    if(resp < 0){
//      buff = "Timed out!";
//    }
//    else buff[0] = b;
//    write(1,buff,strlen(buff));
//    buff = "\n";
//    write(1,buff,strlen(buff));
//  }//