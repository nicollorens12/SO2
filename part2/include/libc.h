/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

#include <sem.h>

#define NULL 0

extern int errno;

int write(int fd, char *buffer, int size);

void itoa(int a, char *b);

void itodec(int a, char *b); // en libc.h

int strlen(char *a);

void perror();

int getpid();

int gettid();

int fork();

void exit();

int yield();

int get_stats(int pid, struct stats *st);

int sys_getKey(char* b, int timeout);

int gotoXY(int x, int y);

int changeColor(int fg, int bg);

int clrscr(char* b);

int threadCreateWithStack( void (*function)(void* arg), int N, void* parameter );

struct sem_t* semCreate(int initial_value);

int semWait(struct sem_t* s);

int semSignal(struct sem_t* s);

int semDestroy(struct sem_t* s);

char* memRegGet(int num_pages);

int memRegDel(char* m);

void SAVE_REGS(void);
void RESTORE_REGS(void);

#endif  /* __LIBC_H__ */
