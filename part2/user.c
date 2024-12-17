#include <libc.h>
#include <screen_helper.h>
#include <dangerous_dave.h>

char key = 'a';
struct sem_t *s2;

void print_thread_sem_1(void* param){

    struct sem_t *s = (struct sem_t *) param;
    
    int tid = gettid();
    char buff[512] = "I'm the new thread number:";
    write(1, buff, strlen(buff));
    itoa(tid, buff);
    write(1, buff, strlen(buff));
    write(1, " ", 1);
    semWait(s);
    key = 'c';
    write(1, &key, 1);
    semSignal(s);
    write(1, "\n", 1);
    semSignal(s2);
    exit();
}

void print_thread_sem_2(void* param){

    struct sem_t *s = (struct sem_t *) param;
    int tid = gettid();

    char buff[512] = "I'm the new thread number:";
    write(1, buff, strlen(buff));

    itoa(tid, buff);
    write(1, buff, strlen(buff));
    write(1, " ", 1);
    semWait(s);
    key = 'k';
    write(1, &key, 1);
    semSignal(s);
    write(1, "\n", 1);
    semSignal(s2);
    exit();
}

void thread_sem_creator(void *param){

  int i = *(int *) param;

    char buff[512] = "I'm the new owner of sem\n";
    write(1, buff, strlen(buff));
    struct sem_t *s = semCreate(1); // Crear semáforo
    s2 = semCreate(0);
    //semWait(s); // Bloquear semáforo
    int t1 = threadCreateWithStack(print_thread_sem_1, 1, s); // Crear thread
    int t2 = threadCreateWithStack(print_thread_sem_2, 1, s); // Crear thread
    semWait(s2);

    char buff2[512] = "I'm the new owner of sem after creating threads and about to exit\n";
    write(1, buff2 ,strlen(buff));
    exit();
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
  char buff[512]= "\nI'm the main thread\n";
  write(1, buff, strlen(buff));
  //int pid = fork();

  int param = 10; // Si es necesario pasar un entero como parámetro
  
  int t = threadCreateWithStack(thread_sem_creator, 10000, &param);
  if(t == -1){
    write(1, "Error creating thread\n", 22);
  }

  char *ptr = memRegGet(4);
  if(ptr == 0){
    write(1, "Error creating memory region\n", 29);
  }
  else{
    write(1, "Memory region created\n", 22);
  }

  *ptr = 'a';
  write(1, "\n", 1);
  write(1, ptr, 1);
  

  ptr += 2000;
  ptr = 'k';
  write(1, "\n", 1);
  write(1, ptr, 1);

  memRegDel(ptr);
  
  while(1) { 
    
  }
}