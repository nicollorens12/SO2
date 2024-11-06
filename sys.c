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

	// Busquem Pagines Fisiques (PF) lliures
	int child_pf_pages[NUM_PAG_DATA];

	for (int i = 0; i < NUM_PAG_DATA; ++i)
	{
		child_pf_pages[i] = alloc_frame();

		// No queden pagines lliures
		if (child_pf_pages[i] < 0)
		{
			// Alliberar pagines fisiques ja assignades
			for (int j = 0; j <= i; ++j)
				free_frame(child_pf_pages[j]);

			// Alliberar PCB del fill --> El tornem a ficar a la cua
			list_add_tail( &(pcb_child->task.list), &freequeue);

			return -ENOMEM;
		}
	}

	// Inicialitzem espai adreces fill
	page_table_entry *pt_child = get_PT(pcb_child);
	page_table_entry *pt_parent = get_PT(pcb_parent);

	// Init KERNEL/SYSTEM --> Podem copiarlo
	for (int i = 0; i < NUM_PAG_KERNEL; ++i)
		pt_child[i] = pt_parent[i];

	// Init CODE --> Podem copiarlo (Necessitem offset d'inici de pagines del codi)
	for (int i = 0; i < NUM_PAG_CODE; ++i)
		pt_child[PAG_LOG_INIT_CODE + i] = pt_parent[PAG_LOG_INIT_CODE + i];

	// Init DATA --> Assignem noves entrades al fill a partir de les PF que hem tret abans (Apunten als frames lliures)
	for (int i = 0; i < NUM_PAG_DATA; ++i)
		pt_child[PAG_LOG_INIT_DATA + i] = child_pf_pages[i];





	// Heretar USER DATA
    int free_pt_entry = -1;
	for (int i = 0; i < NUM_PAG_DATA; ++i)
	{
		//void set_ss_pag(page_table_entry *PT, unsigned page,unsigned frame);
		//void del_ss_pag(page_table_entry *PT, unsigned page);

		// Busquem pagines lliures del pare (es podria fer una rutina que fos mes eficient)	
		// ??? TOTAL_PAGES o nomes NUM_PAG_DATA (de moment m'aseguro que funciona amb TOTAL_PAGES --> molt mes ineficient)
		int is_found = 0;
		for (int j = free_pt_entry + 1; j < TOTAL_PAGES && !is_found; ++j)
    	{
    	    if (pt_parent[j].entry == 0)
    	    {
    	    	is_found = 1;
    	    	free_pt_entry = j;
    	    }
    	}

		// Si s'ha trobat pagina lliure al pare
		if (is_found)
		{
			set_ss_pag(pt_parent, free_pt_entry, child_pf_pages[i]);
			del_ss_pag;
		}
		else
		{
			// Lliurar estructures
		}
	}



	//// Assignar el PID = 1 (Initial task --> Pare de tota la resta)
	//pcb_child->PID = pid_free++;

	// Copiar info fila al fill


  	

  	// Creates the child process
  
  return pcb->PID;
}

void sys_exit()
{  
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
