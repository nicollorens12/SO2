#include <libc.h>
#include <screen_helper.h>
#include <dangerous_dave.h>

void print_thread(int i){
  char buff[512] = "I'm the new thread";
  while(1){
    write(1, buff, strlen(buff));
    itoa(i, buff);
    write(1, buff, strlen(buff));
    write(1, "\n", 1);
  }
  
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
  char buff[512]= "I'm the main thread\n";

  //int t = threadCreateWithStack((void*)print_thread, 1, 1);

  while(1) { 
    //write(1, buff, strlen(buff));
  }
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