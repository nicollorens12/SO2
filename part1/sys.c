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
extern struct list_head blocked;
extern struct task_struct * idle_task;

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

	// Añadir el hijo a la lista de hijos del padre
	struct list_head * list_tmp = &pcb_child->list;
	struct list_head * children_head = &pcb_parent->children;
	list_add_tail(list_tmp, children_head);

	// Copiem el task union del pare al fill
	copy_data(ps_parent, ps_child, sizeof(union task_union)); // Crec que tambe es podria fer servir --> size = KERNEL_STACK_SIZE


	// Directory table del fill
	allocate_DIR(pcb_child);


	// Busquem Pagines Fisiques (PF) lliures + Inicialitzem espai adreces fill
	page_table_entry *pt_child = get_PT(pcb_child);
	page_table_entry *pt_parent = get_PT(pcb_parent);


	// Init KERNEL/SYSTEM --> Podem copiarlo
	for (int i = 0; i < NUM_PAG_KERNEL; ++i)
		pt_child[i] = pt_parent[i];

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
			free_user_pages(pcb_child);
			for (int j = 0; j <= i; ++j)
			{
				free_frame(get_frame(pt_child, PAG_LOG_INIT_DATA + i));
				del_ss_pag(pt_child, PAG_LOG_INIT_DATA + i);
			}	

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
		// Agafo adreces despres Data + Code (a Zeos estan juntes, mirar man) --> Directament despres de les adreces de codi
		// Mapegem pagina temporalment al pare; copiem dades al fill; desfem la relacio	
		set_ss_pag(pt_parent, PAG_LOG_INIT_CODE + NUM_PAG_CODE + i, get_frame(pt_child, PAG_LOG_INIT_DATA + i));
    		copy_data( (PAG_LOG_INIT_DATA + i) << 12, (PAG_LOG_INIT_CODE + NUM_PAG_CODE + i) << 12, PAGE_SIZE);
    		del_ss_pag(pt_parent, PAG_LOG_INIT_CODE + NUM_PAG_CODE + i);
	}


	// flush the TLB to really disable the parent process to access the child pages
	set_cr3(get_DIR(pcb_parent));

	

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
    
	// Assignar el PID
	pcb_child->PID = pid_free++;
	set_quantum(pcb_child, 50);
	pcb_child->pending_unblocks = 0;
	pcb_child->parent = pcb_parent;  // Asignar el proceso padre
	pcb_child->state = ST_READY; // El proceso hijo en ready
	INIT_LIST_HEAD(&pcb_child->children);

	

	// PCB del fill --> El fiquem a la cua de ready
	list_add_tail( &(pcb_child->list), &readyqueue);
  	
  	// Return PID fill
  	return pcb_child->PID;
}


void sys_exit()
{
    struct task_struct *pcb = current();
    page_table_entry *pt_ps = get_PT(pcb);

    // Liberar páginas físicas (PF) (user data) y mapeos de la page table del proceso
    for (int i = 0; i < NUM_PAG_DATA; ++i)
    {
        int pf = get_frame(pt_ps, PAG_LOG_INIT_DATA + i);
        free_frame(pf);              // Liberar el marco de página
        del_ss_pag(pt_ps, PAG_LOG_INIT_DATA + i); // Eliminar el mapeo
    }

    // Cambiar el PID del proceso a -1 para indicar que está terminado
    //pcb->PID = -1;

    //if(pcb->parent != NULL){ NO FUNCIONA, lista de pcb_parent->children se corrompe (intento de acceder a un espacio de paginas incorrecto?)
    //	struct task_struct *pcb_parent = pcb->parent;
    //	//struct task_struct *pcb_parent = list_entry(parent, struct task_struct, parent->list);
    //	struct list_head *pos, *n;
	//	struct list_head *pos_del = NULL;
    //	list_for_each_safe(pos,n,&pcb_parent->children){
    //		struct task_struct *child = list_entry(pos, struct task_struct, list);
//
    //		if(child->PID == pcb->PID){
    //			list_del(pos);
	//			//break;
    //		}
    //	}
	//	if(pos_del != NULL){
	//		list_del(pos);
	//	}
    //}

    // Si el proceso tiene hijos, asignarles un nuevo padre (idle_task)
    //if (!list_empty(&pcb->children)) {
	//	struct list_head *pos, *n;
	//	list_for_each_safe(pos, n, &pcb->children) {
	//		// Obtener el hijo del proceso
	//		struct task_struct *child = list_entry(pos, struct task_struct, list);
	//		
	//		// Asignar a los hijos como hijos de idle_task
	//		child->parent = idle_task;
	//		
	//		// Eliminar el hijo de la lista de hijos del proceso actual
	//		//list_del(pos);
	//		
	//		// Añadir el hijo a la lista de hijos de idle_task
	//		list_add_tail(pos, &idle_task->children);
	//	}
    //}

	//habria que limpiar la pcb a valoes por defecto??

    // Añadir el PCB a la lista de procesos libres
    list_add_tail(&(pcb->list), &freequeue);

    // Utilizar la interfaz del planificador para seleccionar un nuevo proceso para ejecutar
    // y hacer un cambio de contexto.
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

int sys_block(){
	if(current()->pending_unblocks > 0) current()->pending_unblocks--;
	else if (current()->pending_unblocks == 0){
		update_process_state_rr(current(), &blocked);
		current()->state = ST_BLOCKED;
		sched_next_rr();
	}
	else return -1;

	return 1;
}

int sys_unblock(int PID) {
    if (PID < 0) {
        return -1;
    }

    struct task_struct *parent = current();

    // Verificar si el proceso actual tiene hijos
    struct list_head *pos;
    list_for_each(pos, &parent->children) {
        struct task_struct *child = list_entry(pos, struct task_struct, list);
        
        if (child->PID == PID) {
            if (child->state != ST_BLOCKED) {
                child->pending_unblocks++;
            } else {
                // Eliminar el proceso hijo de la lista de bloqueados
                list_del(&child->list);

                // Cambiar el estado del hijo a listo (TASK_READY)
                child->state = ST_READY;

                // Añadir el proceso hijo a la lista de procesos listos
                list_add_tail(&child->list, &readyqueue);
				update_process_state_rr(&child, &readyqueue);
				sched_next_rr();
            }
            return 0; 
        }
    }
    return -1; 
}



