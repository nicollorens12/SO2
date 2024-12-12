/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

#include <sem.h>

extern int errno;

int write(int fd, char *buffer, int size);

void itoa(int a, char *b);

int strlen(char *a);

void perror();

int getpid();

int fork();

void exit();

int yield();

int get_stats(int pid, struct stats *st);

int gotoXY(int x, int y);

int changeColor(int fg, int bg);

int clrscr(char* b);

struct sem_t* semCreate(int initial_value);

int semWait(struct sem_t* s);

int semSignal(struct sem_t* s);

int semDestroy(struct sem_t* s);

void SAVE_REGS(void);
void RESTORE_REGS(void);

#endif  /* __LIBC_H__ */
