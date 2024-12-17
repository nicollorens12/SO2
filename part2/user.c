#include <libc.h>
#include <screen_helper.h>
#include <dangerous_dave.h>

char key;

void child_thread_funcs(void *param){
  
    struct sem_t *sem = (struct sem_t *) param;

    char buff[512] = "I'm the new thread of a thread with TID:";
    write(1, buff, strlen(buff));

    char buff2[512];
    itoa(gettid(), buff2);
    write(1, buff2, strlen(buff2));
    write(1, "\n", 1);

    semWait(sem);
    write(1, "Printing char a", 15);
    key = 'a';
    write(1, &key, 1);
    semSignal(sem);


    exit();
}

void child_thread_funcs_2(void *param){
  
    struct sem_t *sem = (struct sem_t *) param;

    char buff[512] = "I'm the new thread of a thread with TID:";
    write(1, buff, strlen(buff));

    char buff2[512];
    itoa(gettid(), buff2);
    write(1, buff2, strlen(buff2));
    write(1, "\n", 1);

    semWait(sem);
    write(1, "Printing char b", 15);
    key = 'b';
    write(1, &key, 1);
    semSignal(sem);


    exit();
}

void create_thread(void *param){
  int i = *(int *) param;
  char buff[128] = "I'm the new thread father\n";
  write(1, buff, strlen(buff));

  struct sem_t *sem = semCreate(1);

  
    if(threadCreateWithStack(child_thread_funcs, 1, &sem) == -1){
      write(1, "Error creating thread\n", 22);
    }
    if(threadCreateWithStack(child_thread_funcs_2, 1, &sem) == -1){
      write(1, "Error creating thread\n", 22);
    }
  
  while(1);

  exit();
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

  char *reg = memRegGet(5);
  write(1, "\n", 1);
  *reg = 'a';
  write(1, reg, 1);
  memRegDel(reg);
  
  while(1) { 
    
  }
}