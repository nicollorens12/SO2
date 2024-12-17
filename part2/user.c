#include <libc.h>
#include <screen_helper.h>
#include <dangerous_dave.h>

void child_thread_funcs(void *param){
  
    int i = *(int *) param;
    char buff[512] = "I'm the new thread of a thread with TID:\n";
    write(1, buff, strlen(buff));

    char buff2[512];
    itoa(gettid(), buff2);
    write(1, buff2, strlen(buff2));
}

void create_thread(void *param){
  int i = *(int *) param;
  char buff[128] = "I'm the new thread father\n";
  write(1, "buff", 4);

  for(int j = 0; j < i; j++){
    if(threadCreateWithStack(child_thread_funcs, 1, &i) == -1){
      write(1, "Error creating thread\n", 22);
    }
  }
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
  char buff[512]= "\nI'm the main thread\n";
  write(1, buff, strlen(buff));

  int param = 3; // Si es necesario pasar un entero como parámetro
  
  int t = threadCreateWithStack(create_thread, 10000, &param);
  if(t == -1){
    write(1, "Error creating thread\n", 22);
  }

  t = threadCreateWithStack(create_thread, 10, &param);
  if(t == -1){
    write(1, "Error creating thread\n", 22);
  }
  
  while(1) { 
    
  }
}