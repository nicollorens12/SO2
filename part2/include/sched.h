/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>
#include <stats.h>
#include <sem.h>


#define NR_TASKS      10
#define KERNEL_STACK_SIZE	1024
#define NUM_SEM 10

enum state_t { ST_RUN, ST_READY, ST_BLOCKED };
enum wake_t {SEM_SIG = 1, SEM_DES = -1};

struct task_struct {
  int PID;			/* Process ID. This MUST be the first field of the struct. */
  int TID;      /* Thread ID */
  page_table_entry * dir_pages_baseAddr;
  struct list_head list;	/* Task struct enqueuing */
  struct list_head list_ordered; /* Task struct for ordered list */
  int register_esp;		/* position in the system stack */
  enum state_t state;		/* State of the process */
  int total_quantum;		/* Total quantum of the process */
  struct stats p_stats;		/* Process stats */
  int expiring_time;

  /* Nuevo para threads */
  void *user_stack_base;       /* Dirección base del stack del usuario */
  int num_stack_pages;         /* Número de páginas del stack dinámico (N)*/
  struct list_head threads;     /* Lista de threads del proceso */
  struct list_head list_thread;  /* Entrada de la lista de threads del proceso*/
  int wake_reason;            
};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};

extern union task_union protected_tasks[NR_TASKS+2];
extern union task_union *task; /* Vector de tasques */
extern struct task_struct *idle_task;
extern struct sem_t sem_list[NUM_SEM];

#define KERNEL_ESP(t)       	(DWord) &(t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP       	KERNEL_ESP(&task[1])

extern struct list_head freequeue;
extern struct list_head readyqueue;

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

void schedule(void);

struct task_struct * current();

void task_switch(union task_union*t);
void switch_stack(int * save_sp, int new_sp);

void sched_next_rr(void);

void force_task_switch(void);

struct task_struct *list_head_to_task_struct(struct list_head *l);

int allocate_DIR(struct task_struct *t);

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;

/* Headers for the scheduling policy */
void sched_next_rr();
void update_process_state_rr(struct task_struct *t, struct list_head *dest);
int needs_sched_rr();
void update_sched_data_rr();

void init_stats(struct stats *s);

#endif  /* __SCHED_H__ */
