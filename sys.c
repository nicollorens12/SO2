/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

extern int zeos_ticks;

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int ret_from_fork()
{
	return 0;
}

int sys_fork()
{
  	// Comprobar que hi ha algun PCB lliure (la llista no es buida)
  	if (list_empty(&freequeue))
  		return -ENOSPC; // No hi ha espai lliure --> Consultar si deuria ser aquest el codi d'error (ENOMEM?)

  	// Obtenir el primer element de la llista de lliures
	struct list_head* item = list_first(&freequeue);
	// Eliminem entrada de la llista
	list_del(item);

	// Obtenir punter al task_struct (=== PCB)
	struct task_struct* pcb_child = list_head_to_task_struct(item);
	// Obtenim el task union del proces per treballar amb la seva pila
	union task_union* ps_child = (union task_union*)pcb_child;

	// Obtenim el pcb del process actual (pare)
	struct task_struct* pcb_parent = current();
	// Task union del pare
	union task_union* ps_parent = (union task_union*)pcb_parent;

	// Copiem el task union del pare al fill
	copy_data(ps_parent, ps_child, sizeof(union task_union)); // Crec que tambe es podria fer servir --> size = KERNEL_STACK_SIZE


	// Directory table del fill
	allocate_DIR(pcb_child);


	// Busquem Pagines Fisiques (PF) lliures + Inicialitzem espai adreces fill
	page_table_entry *pt_child = get_PT(pcb_child);
	page_table_entry *pt_parent = get_PT(pcb_parent);


	// Init KERNEL/SYSTEM --> Podem copiarlo
	for (int i = 0; i < NUM_PAG_KERNEL; ++i)
		set_ss_pag(pt_child, i, get_frame(pt_parent, i));
		// pt_child[i] = pt_parent[i];  // 1 - Comentari del correu

	// Init CODE --> Podem copiarlo (Necessitem offset d'inici de pagines del codi)
	for (int i = 0; i < NUM_PAG_CODE; ++i)
		set_ss_pag(pt_child, PAG_LOG_INIT_CODE + i, get_frame(pt_parent, PAG_LOG_INIT_CODE + i));

	// Init DATA --> Assignem noves entrades al fill a partir de les PF/frames lliures
	for (int i = 0; i < NUM_PAG_DATA; ++i)
	{
		int free_pf = alloc_frame();

		// No queden pagines lliures
		if (free_pf < 0)
		{
			// Alliberar pagines fisiques ja assignades
			free_user_pages(pcb_child); // --> m' "estalvio" bucle free_frame ?? Preguntar si aixi ja esta be o he de fer el bucle de free_frame

			// Alliberar PCB del fill --> El tornem a ficar a la cua
			list_add_tail( &(pcb_child->list), &freequeue);

			return -ENOMEM;
		}
		
		// Assignem la pagina fisica
		set_ss_pag(pt_child, PAG_LOG_INIT_DATA + i, free_pf);
	}


	// Heretar USER DATA
	for (int i = 0; i < NUM_PAG_DATA; ++i)
	{
		// Busquem pagines lliures del pare (es podria fer una rutina que fos mes eficient o adient)	
		/*
			- He de buscar a totes les pagines?? (Les de kernel no crec que calgui)
			- No es podria fer servir un punt fixe a l'espai entre PAG_CODE i PAG_DATA?
			- De moment agafare les que hi hagi mes enlla de PAG_DATA 

			- En cas que no hi hagues pagines lliures al proces pare hauria de retornar error (i lliberar estructures) --> de moment no ho toquem
		*/

		// Mapegem pagina temporalment al pare; copiem dades al fill; desfem la relacio	
		set_ss_pag(pt_parent, PAG_LOG_INIT_DATA + NUM_PAG_DATA + i, get_frame(pt_child, PAG_LOG_INIT_DATA + i));
    	copy_data(&(pt_parent[PAG_LOG_INIT_DATA + NUM_PAG_DATA + i]), &(pt_child[PAG_LOG_INIT_DATA + i]), PAGE_SIZE);
    	// copy_data( (PAG_LOG_INIT_DATA + NUM_PAG_DATA + i) << 12, (PAG_LOG_INIT_DATA + i) << 12, PAGE_SIZE);  // 2 - Comentari del correu
    	del_ss_pag(pt_parent, PAG_LOG_INIT_DATA + NUM_PAG_DATA + i);
	}


	// flush the TLB to really disable the parent process to access the child pages
	set_cr3(get_DIR(pcb_parent));

	// Assignar el PID
	pcb_child->PID = pid_free++;

	// Initialize the fields of the task_struct that are not common to the child.
	/* 

	NO CAL QUE TOQUI CAP ALTRE DE MOMENT NO??

	int PID;	--> ja l'he modificat
  	page_table_entry * dir_pages_baseAddr; --> ja l'he estat manipulant
  	struct list_head list; --> Ja l'he tret de free i mes tard el ficare a ready
  	unsigned int kernel_esp; --> Quan prepari stack per ret_from_fork(), ja el modificare

	*/

  	// Prepare the child stack so that a task_switch call on this PCB restores the process execution
  	// A la pila hi ha: CTX HW (5) + CTX SW (11) + @ret_handler (1) = 17 entrades
	ps_child->stack[KERNEL_STACK_SIZE - 19] = 0; // Fake %ebp
	ps_child->stack[KERNEL_STACK_SIZE - 18] = &ret_from_fork; // Substituim %ebp --> @ret_from_fork

	// Fem que esp apunti al cim de la pila --> Aixi ja deuria estar llest per quan faci el task_switch
    pcb_child->kernel_esp = &(ps_child->stack[KERNEL_STACK_SIZE - 19]);
    

	// PCB del fill --> El fiquem a la cua de ready
	list_add_tail( &(pcb_child->list), &readyqueue);
  	
  	// Return PID fill
  	return pcb_child->PID;
}


void sys_exit()
{  
	struct task_struct *pcb = current();
	page_table_entry *pt_ps = get_PT(pcb);

	// Alliberar PF (user data) i mapejos a la page table del proces
	for (int i = 0; i < NUM_PAG_DATA; ++i)
	{
		int pf = get_frame(pt_ps, PAG_LOG_INIT_DATA + i);
		free_frame(pf);
		del_ss_pag(pt_ps, PAG_LOG_INIT_DATA + i);
	}

	// Cambiem el PCB
	pcb->PID = -1;

	// Cal que retoqui dir_pages_baseAddr i kernel_esp?? --> Diria que no cal, ja que al crear un nou proces amb fork aquests es 'machaquen'

	// Posem la pcb a la llista de lliures
	list_add_tail( &(pcb->list), &freequeue);

	// Use the scheduler interface to select a new process to be executed and make a context switch	
	sched_next_rr();
}





#define BUFFER_SIZE 512
char buffer_content[BUFFER_SIZE];

// fd: file descriptor. In this delivery it must always be 1.
// buffer: pointer to the bytes.
// size: number of bytes.
// return ’ Negative number in case of error (specifying the kind of error) and
// 			the number of bytes written if OK.
int sys_write(int fd, char * buffer, int size){

	// CHECK USER PARAMS
	// Check fd (0: no hay error, < 0: id del error)
	int id_error = check_fd(fd, ESCRIPTURA);
	if(id_error < 0) return id_error;

	// Check buffer 
	if(buffer == NULL) return -EFAULT; // TODO - No se que devolver

	// Check size
	if(size < 0) return -EINVAL; // TODO - No se que devolver


	
	int bytes = size;
	int bytes_written; 

	while(bytes > BUFFER_SIZE){
		// Copy data from user
		copy_from_user(buffer, buffer_content, BUFFER_SIZE);
		// Device routine --> write on console
		bytes_written = sys_write_console(buffer_content, BUFFER_SIZE);
		
		buffer = buffer+BUFFER_SIZE; //potser cal sumar bytes_written?
		bytes -= bytes_written;
	}

	// Imprimir los restantes
	copy_from_user(buffer, buffer_content, bytes);
	bytes_written = sys_write_console(buffer_content, bytes);
	bytes -= bytes_written;	


	return size - bytes;
}

int sys_gettime(){
	return zeos_ticks;
}
