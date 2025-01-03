/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>

#define NR_TASKS      10
#define KERNEL_STACK_SIZE	1024

enum state_t { ST_RUN, ST_READY, ST_BLOCKED };

struct task_struct {
  int PID;			/* Process ID. This MUST be the first field of the struct. */
  page_table_entry * dir_pages_baseAddr;
  struct list_head list; // Punter/Posicio a la llista de freequeue (pcb lliure) 
  unsigned int kernel_esp; // %esp: Apunta al top de la pila de sistema (faig servir int perque es CPU de 32 bits, si no long)
  int quantum; // max time that the process should spend without stop on running
  int pending_unblocks;
  enum state_t state;

  struct task_struct *parent;          /* Puntero al proceso padre */
  struct list_head children;           /* Lista de hijos */
};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};

extern union task_union task[NR_TASKS]; /* Vector de tasques */


#define KERNEL_ESP(t)       	(DWord) &(t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP       	KERNEL_ESP(&task[1])


extern struct list_head freequeue;
extern struct list_head readyqueue;

extern int pid_free; // Next pid available to asign on fork

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

struct task_struct * current();

void inner_task_switch(union task_union *new);
void task_switch(union task_union*t);

struct task_struct *list_head_to_task_struct(struct list_head *l);

int allocate_DIR(struct task_struct *t);

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;

/* Headers for the scheduling policy */
void schedule(void);

void sched_next_rr();
void update_process_state_rr(struct task_struct *t, struct list_head *dest);
int needs_sched_rr();
void update_sched_data_rr();
void change_process(); //Funcion auxiliar para quitar un proceso de la CPU, ponerlo en ready y meter al siguiente
/* Headers for quantum */
int get_quantum (struct task_struct *t);
void set_quantum (struct task_struct *t, int new_quantum);

#endif
