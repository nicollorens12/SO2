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

int pid_free = 2;
int quantum_ticks;

int task1_quantum = 6;
int idle_quantum = 3;

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
	set_quantum(pcb, idle_quantum);

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
	set_quantum(pcb, task1_quantum);

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

void schedule(){
	update_sched_data_rr();
	if(needs_sched_rr()) {
		update_process_state_rr(current(), &readyqueue);
		sched_next_rr();
	}
}

void sched_next_rr(){
	struct task_struct *next;

	// Si hay procesos en la lista de ready
	if(!list_empty(&readyqueue)){
		// Nos quedamos con el primero de la lista y lo eliminamos de esta.
		struct list_head *lf = list_first(&readyqueue);
		list_del(lf);
		next = list_head_to_task_struct(lf);
	}
	else{
		// Si la lista de ready esta vacía, se debe ejecutar el proceso idle.
		next = idle_task;
	}

	// Se pone el estado del proceso a RUN. - Creo que no es necesario
	// next->state=ST_RUN;

	// Se asigna la variable global de quantum el quantum del proceso siguiente.
	quantum_ticks = get_quantum(next);

	char* buff;
	char pid_str[10];
	itoa(current()->PID, pid_str, 10);
	buff = "Current pid before switch is: ";
	printk(buff);
	printk(pid_str);
	inner_task_switch(next);
	itoa(current()->PID, pid_str, 10);
	buff = "Current pid after switch is: ";
	printk(buff);
	printk(pid_str);
}

void update_process_state_rr(struct task_struct *t, struct list_head *dest){
	if(dest == NULL){ // Esto significa que el proceso esta en Running
		list_add_tail(&(t->list), dest);
	}
	else{
		list_del(&(t->list));
		list_add_tail(&(t->list), dest);
	}
}

int needs_sched_rr(){
	if(quantum_ticks > 0) return 0;

	if(list_empty(&readyqueue)){ // En el caso de que no tengamos procesos ready para ponerse en CPU
		// Aun asi, probablemente no se usa ya que tenemos el idle_task 
		quantum_ticks = get_quantum(current());
		return 0;
	}
	return 1;
}

void update_sched_data_rr(){
	// Decrementamos en uno el quantum_ticks ya que schedule() 
	// que es la funcion que llama a esta funcion, se llama cada vez que se hace un tick del clock de la CPU
	--quantum_ticks;
	//char *buff;
	//char pid_str[10];
	//char ticks_str[10];
	//itoa(current()->PID, pid_str, 10);
	//itoa(quantum_ticks, ticks_str, 10);
//
	//buff = "Quantum ticks left for process ";
	//printk(buff);
	//printk(pid_str);
	//buff = " is ";
	//printk(buff);
	//printk(ticks_str);
}

int get_quantum (struct task_struct *t){
	return t->quantum;
}

void set_quantum (struct task_struct *t, int new_quantum){
	t->quantum = new_quantum;
}

