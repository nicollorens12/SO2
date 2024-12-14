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
  char buff[512]= "\nI'm the main thread\n";
   write(1, buff, strlen(buff));

  char *reg = memRegGet(4);
  char *reg_aux1, *reg_aux2, *reg_aux3;
  reg_aux1 = reg;
  reg_aux2 = reg + 1;
  reg_aux3 = reg + 2001;

  *reg_aux1 = 'a';
  write(1, reg_aux1, 1); // Write 'a'
  (*reg_aux2) = 'b';
  (*reg_aux3) = 'c';

  
  write(1, "\n", 1);
  write(1, reg_aux2, 1); // Write 'b'
  write(1, "\n", 1);
  write(1, reg_aux3, 1); // Write 'c'
  write(1, "\n", 1);

  memRegDel(reg);

  reg_aux1 = 'k';
  write(1, reg_aux1, 1); // Write 'a'



  
  while(1) { 
    
  }
}