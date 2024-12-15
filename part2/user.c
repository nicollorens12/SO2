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

void print_thread_sem(struct sem_t *s){
    char buff[512] = "I'm the new thread about to block\n";
    write(1, buff, strlen(buff));
    semWait(s);
    int tid = gettid();
    write(1, "I'm the new thread after blocking number:", strlen(buff));
    itoa(tid, buff);
    write(1, buff, strlen(buff));
    write(1, "\n", 1);
    exit();
}

void thread_sem_creator(int i){
  char buff[512] = "I'm the new owner of sem\n";
    write(1, buff, strlen(buff));
    struct sem_t *s = semCreate(1); // Crear semáforo
    semWait(s); // Bloquear semáforo
    int t1 = threadCreateWithStack(print_thread_sem, 1, s); // Crear thread
    int t2 = threadCreateWithStack(print_thread_sem, 1, s); // Crear thread
    write(1, "I'm the new owner of sem after creating threads and about to exit\n", strlen(buff));
    exit();
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
  // char buff[512]= "\nI'm the main thread\n";

  // int pid = fork();

  // int param = 10; // Si es necesario pasar un entero como parámetro
  
  // int t = threadCreateWithStack(thread_sem_creator, 1, &param);
  // write(1, buff, strlen(buff));



  game_loop();

  while(1);
}