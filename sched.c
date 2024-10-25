/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

#if 1
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;
struct list_head freequeue;
struct list_head readyqueue;
struct task_struct * idle_task;


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	;
	}
}

void init_idle (void)
{
	// Obtenir el primer element de la llista de lliures
	struct list_head* item = list_first(&freequeue);
	// Eliminem entrada de la llista
	list_del(item);

	// Obtenir punter al task_struct (=== PCB)
	struct task_struct* pcb = list_head_to_task_struct(item);
	// Assignar el PID = 0
	pcb->PID = 0;

	// Initialize field dir_pages_baseAaddr with a new directory to store the process address space
	allocate_DIR(pcb);

	/* Initialize an execution context for the process to execute cpu_idle function when it gets assigned the cpu */
	/* -- dynamic link -- */
	// Obtenim el task union del proces per treballar amb la seva pila
	union task_union* ps_union = (union task_union*)pcb;

	ps_union->stack[KERNEL_STACK_SIZE - 1] = (unsigned long)&cpu_idle; // address of the cpu_idle function (@ret: es ella mateixa, sempre es crida a si mateix)
	ps_union->stack[KERNEL_STACK_SIZE - 2] = 0; // %ebp value

	pcb->kernel_esp = &(ps_union->stack[KERNEL_STACK_SIZE - 2]); // Posem valor a %esp (stack pointer)

	// Inicialitzar variable global idle_task amb el pcb
	idle_task = pcb;
}

void init_task1(void)
{
	// Obtenir el primer element de la llista de lliures
	struct list_head* item = list_first(&freequeue);
	// Eliminem entrada de la llista
	list_del(item);

	// Obtenir punter al task_struct (=== PCB)
	struct task_struct* pcb = list_head_to_task_struct(item);
	// Assignar el PID = 1 (Initial task --> Pare de tota la resta)
	pcb->PID = 1;

	// Initialize field dir_pages_baseAaddr with a new directory to store the process address space
	allocate_DIR(pcb);

	// Inicialitzar espai d'adreces del proces
	// Alocata pagines fisiques per contenir l'espai d'adreces de l'usuari (tant codi com pagines de dades) i 
	// afegeix a la taula de pagines la traduccio logica a fisica d'aquestes pagines
	set_user_pages(pcb);


	// Obtenim el task union del proces per treballar amb la seva pila
	union task_union* ps_union = (union task_union*)pcb;

	// Fem que la tss apunti al system stack del nou proces
	// Al .h tenim definida una funcio que fa el mateix que: &(ps_union->stack[KERNEL_STACK_SIZE])
	tss.esp0 = KERNEL_ESP(ps_union);

	// Ara MSR que apunti tambe al system stack del nou proces
	writeMSR(0x175, tss.esp0);

	// Set its page directory as the current page directory in the system, by using the set_cr3
	set_cr3(pcb->dir_pages_baseAddr);
}


void init_sched()
{
	// FREEQUEUE: Free/Available PCBs
	INIT_LIST_HEAD(&freequeue); // Incialitzar la llista de lliures buida
	for (int i = 0; i < NR_TASKS; ++i) {
		list_add_tail(&task[i].task.list, &freequeue);
	}

	// READY
	INIT_LIST_HEAD(&readyqueue);
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

void inner_task_switch(union task_union *new){
	set_cr3(get_DIR(&(new->task))); //  Cambio de la paginacion a la del nuevo proceso
	
	tss.esp0 = KERNEL_ESP((union task_union *)new); 
	writeMSR(0x175, (int) tss.esp0);

	current()->kernel_esp = get_ebp();

	set_esp(new->task.kernel_esp);
}
