/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <p_stats.h>

#include <errno.h>

#include <interrupt.h>

#define LECTURA 0
#define ESCRIPTURA 1

extern struct list_head blocked;
extern struct list_head getKeyBlocked;
extern struct list_head key_blockedqueue;
extern struct list_head threads;

extern int pending_key;

void * get_ebp();

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; 
  if (permissions!=ESCRIPTURA) return -EACCES; 
  return 0;
}

void user_to_system(void)
{
  update_stats(&(current()->p_stats.user_ticks), &(current()->p_stats.elapsed_total_ticks));
}

void system_to_user(void)
{
  update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
}

int sys_ni_syscall()
{
	return -ENOSYS; 
}

int sys_getpid()
{
	return current()->PID;
}

int global_PID=1000;
int global_TID=0;

int ret_from_fork()
{
  return 0;
}

int sys_fork(void)
{
  struct list_head *lhcurrent = NULL;
  union task_union *uchild;
  
  /* Any free task_struct? */
  if (list_empty(&freequeue)) return -ENOMEM;

  lhcurrent=list_first(&freequeue);
  
  list_del(lhcurrent);
  
  uchild=(union task_union*)list_head_to_task_struct(lhcurrent);
  
  /* Copy the parent's task struct to child's */
  copy_data(current(), uchild, sizeof(union task_union));
  
  /* new pages dir */
  allocate_DIR((struct task_struct*)uchild);
  
  /* Allocate pages for DATA+STACK */
  int new_ph_pag, pag, i;
  page_table_entry *process_PT = get_PT(&uchild->task);
  for (pag=0; pag<NUM_PAG_DATA; pag++)
  {
    new_ph_pag=alloc_frame();
    if (new_ph_pag!=-1) /* One page allocated */
    {
      set_ss_pag(process_PT, PAG_LOG_INIT_DATA+pag, new_ph_pag);
    }
    else /* No more free pages left. Deallocate everything */
    {
      /* Deallocate allocated pages. Up to pag. */
      for (i=0; i<pag; i++)
      {
        free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
        del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
      }
      /* Deallocate task_struct */
      list_add_tail(lhcurrent, &freequeue);
      
      /* Return error */
      return -EAGAIN; 
    }
  }

  /* Copy parent's SYSTEM and CODE to child. */
  page_table_entry *parent_PT = get_PT(current());
  for (pag=0; pag<NUM_PAG_KERNEL; pag++)
  {
    set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));
  }
  for (pag=0; pag<NUM_PAG_CODE; pag++)
  {
    set_ss_pag(process_PT, PAG_LOG_INIT_CODE+pag, get_frame(parent_PT, PAG_LOG_INIT_CODE+pag));
  }
  /* Copy parent's DATA to child. We will use TOTAL_PAGES-1 as a temp logical page to map to */
  for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; pag++)
  {
    /* Map one child page to parent's address space. */
    set_ss_pag(parent_PT, pag+NUM_PAG_DATA, get_frame(process_PT, pag));
    copy_data((void*)(pag<<12), (void*)((pag+NUM_PAG_DATA)<<12), PAGE_SIZE);
    del_ss_pag(parent_PT, pag+NUM_PAG_DATA);
  }
  /* Deny access to the child's memory space */
  set_cr3(get_DIR(current()));

  uchild->task.PID=++global_PID;
  uchild->task.state=ST_READY;
  uchild->task.expiring_time = -1;

  int register_ebp;		/* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(uchild);

  uchild->task.register_esp=register_ebp + sizeof(DWord);

  DWord temp_ebp=*(DWord*)register_ebp;
  /* Prepare child stack for context switch */
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=(DWord)&ret_from_fork;
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=temp_ebp;

  /* Set stats to 0 */
  init_stats(&(uchild->task.p_stats));

  /* Queue child process into readyqueue */
  uchild->task.state=ST_READY;
  list_add_tail(&(uchild->task.list), &readyqueue);
  
  return uchild->task.PID;
}

#define TAM_BUFFER 512

int sys_write(int fd, char *buffer, int nbytes) {
char localbuffer [TAM_BUFFER];
int bytes_left;
int ret;

	if ((ret = check_fd(fd, ESCRIPTURA)))
		return ret;
	if (nbytes < 0)
		return -EINVAL;
	if (!access_ok(VERIFY_READ, buffer, nbytes))
		return -EFAULT;
	
	bytes_left = nbytes;
	while (bytes_left > TAM_BUFFER) {
		copy_from_user(buffer, localbuffer, TAM_BUFFER);
		ret = sys_write_console(localbuffer, TAM_BUFFER);
		bytes_left-=ret;
		buffer+=ret;
	}
	if (bytes_left > 0) {
		copy_from_user(buffer, localbuffer,bytes_left);
		ret = sys_write_console(localbuffer, bytes_left);
		bytes_left-=ret;
	}
	return (nbytes-bytes_left);
}


extern int zeos_ticks;

int sys_gettime()
{
  return zeos_ticks;
}

void sys_exit()
{  
  int i;

  page_table_entry *process_PT = get_PT(current());

  // Deallocate all the propietary physical pages
  for (i=0; i<NUM_PAG_DATA; i++)
  {
    free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
    del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
  }
  
  /* Free task_struct */
  list_add_tail(&(current()->list), &freequeue);
  
  current()->PID=-1;
  
  /* Restarts execution of the next process */
  sched_next_rr();
}

/* System call to force a task switch */
int sys_yield()
{
  force_task_switch();
  return 0;
}

extern int remaining_quantum;

int sys_get_stats(int pid, struct stats *st)
{
  int i;
  
  if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT; 
  
  if (pid<0) return -EINVAL;
  for (i=0; i<NR_TASKS; i++)
  {
    if (task[i].task.PID==pid)
    {
      task[i].task.p_stats.remaining_ticks=remaining_quantum;
      copy_to_user(&(task[i].task.p_stats), st, sizeof(struct stats));
      return 0;
    }
  }
  return -ESRCH; /*ESRCH */
}

int compare_expiring_time(void *a, void  *b)
{
  struct task_struct *t1 = (struct task_struct *)a;
  struct task_struct *t2 = (struct task_struct *)b;

  return t1->expiring_time < t2->expiring_time;
}


int sys_getKey(char* b, int timeout){
  if(timeout <= 0) return 1;
  if(list_empty(&key_blockedqueue)){ // Hay que comprobar, ya que si hay alguien bloqueado, 
      if(pending_key == 0){
          if(get_chars_pending_cb(&circular_buffer) > 0){
              read_element_cb(&circular_buffer,b);
              return 0;
          }
      }
      
  }
    int expiring_time = sys_gettime() + (timeout * 1000); //Nunca podra ser negativo
    update_process_state_rr(current(), &key_blockedqueue);
    current()->state = ST_BLOCKED;
    current()->expiring_time = expiring_time;
    list_add_ordered(&current()->list_ordered, &getKeyBlocked);

    sched_next_rr();

    if(current()->expiring_time == -1) return -1;
    read_element_cb(&circular_buffer,b);
    --pending_key;
  
  
  return 1;
}

int sys_gotoXY(int x, int y)
{
  // Check x & y inside screen range (80,25)
  if (x < 0 || x > 80 || y < 0 || y > 25)
    return -ESCRXY; /* Invalid argument */

  move_cursor((char)x, (char)y);

  return 0;
}

int sys_changeColor(int fg, int bg)
{
  // Rang: (000 -> 111 per bg), (0000 -> 1111 per fg)
  if (fg < 0b0000 || fg > 0b1111 || bg < 0b000 || bg > 0b111)
    return -ESCRCOLOR; /* Invalid argument */

  change_color(fg, bg);
  return 0;
}

int sys_clrscr(char* b)
{
  // Gestio errors: La matriu es fora de l'espai d'adreces de l'usuari
  if ( b < L_USER_START || (b + sizeof(b)) > USER_ESP)
    return EFAULT;  /* Bad address */

  // Moure el cursor a l'inici de la pantalla
  move_cursor(0, 0);

  for (int i = 0; i < 25; ++i)
  {
    for (int j = 0; j < 80; ++j)
    {
      if (b != NULL)
        // Suposant que la matriu es guarda per files: desplacament de fila + access columna
        // Part alta: color; Part baixa: char
        printc_raw(b[i*80*2 + j*2 + 1] << 8 | b[i*80*2 + j*2 + 0]); 
      else 
        printc_raw(0);
    }
  }

  return 1;
}

void thread_exit() {
    struct task_struct *current_thread = current();

    // Liberar el stack del usuario
    for (int i = 0; i < current_thread->num_stack_pages; i++) {
        unsigned long page = (unsigned long)current_thread->user_stack_base + i * PAGE_SIZE;
        free_frame(get_frame(get_PT(current_thread), page));
        del_ss_pag(get_PT(current_thread), page);
    }

    current_thread->user_stack_base = NULL;
    current_thread->num_stack_pages = 0;

    schedule();  // Cambiar al siguiente thread
}


struct task_struct *allocate_task_struct(union task_union **task)
{
  struct list_head *lh = list_first(&freequeue);  // Obtenemos la primera tarea libre
  if (lh == NULL) {
    return NULL;  // No hay más tareas disponibles
  }

  list_del(lh);  // La eliminamos de la lista de tareas libres
  *task = (union task_union*)list_head_to_task_struct(lh);  // Convertimos la lista a un task_struct
  return &(*task)->task;
}

void* allocate_user_stack(int N) {
    page_table_entry *process_pt = get_PT(current());

    struct list_head *lt = list_tail(&threads);
    union task_union *last_thread = list_head_to_task_struct(lt);
    int *last_thread_stack = last_thread->task.user_stack_base;

    int base_pag = last_thread_stack >> 12;

    for(int i = 0; i < N; ++i){
        int new_ph_pag = alloc_frame();
        if(new_ph_pag != -1){
            set_ss_pag(process_pt, last_thread_stack+i , new_ph_pag);
        } 
        else {
            for(int j = 0; j < i; ++j){
                free_frame(get_frame(process_pt, USER_STACK_BASE_PAGES - j - 1));
                del_ss_pag(process_pt, USER_STACK_BASE_PAGES - j - 1);
            }
            return NULL;
        }
    }
}

void free_task_struct(union task_union *task)
{
  struct list_head *lh = &task->task.list;
  list_add_tail(lh, &freequeue);  // Añadimos la tarea de vuelta a la lista de tareas libres
}

void copy_tcb(union task_union *new){
    union task_union *father = (union task_union *)current();
    new->task.register_esp = father->task.register_esp;
    new->task.dir_pages_baseAddr = father->task.dir_pages_baseAddr;
    new->task.total_quantum = father->task.total_quantum;
    new->task.state = father->task.state;
    new->task.expiring_time = father->task.expiring_time;
    new->task.PID = father->task.PID;
    new->task.user_stack_base = father->task.user_stack_base;
    new->task.num_stack_pages = father->task.num_stack_pages;
    new->task.user_esp = father->task.user_esp;
    new->task.p_stats.remaining_ticks = father->task.p_stats.remaining_ticks;
    new->task.p_stats.elapsed_total_ticks = father->task.p_stats.elapsed_total_ticks;
    new->task.p_stats.user_ticks = father->task.p_stats.user_ticks;
    new->task.p_stats.system_ticks = father->task.p_stats.system_ticks;
    new->task.p_stats.blocked_ticks = father->task.p_stats.blocked_ticks;
    new->task.p_stats.ready_ticks = father->task.p_stats.ready_ticks;
    new->task.p_stats.elapsed_total_ticks = father->task.p_stats.elapsed_total_ticks;
}


int sys_threadCreateWithStack(void (*start_routine)(void), int N, void *argument ) {
    
    struct list_head *lhcurrent = NULL;
    union task_union *new_thread;
    
    /* Any free task_struct? */
    if (list_empty(&freequeue)) return -ENOMEM;

    lhcurrent=list_first(&freequeue);
    
    list_del(lhcurrent);
    
    new_thread=(union task_union*)list_head_to_task_struct(lhcurrent);

    copy_tcb(new_thread);

    if (!new_thread) return -ENOMEM;

    // Asignar el stack para el hilo
    void *stack_base = allocate_user_stack(N);
    if (!stack_base) {
        free_task_struct(new_thread); // Liberar la estructura si no se pudo asignar el stack
        return -ENOMEM; // Error al asignar el stack
    }

    // Ajustar TCB
    new_thread->user_stack_base = stack_base;
    new_thread->num_stack_pages = N;
    new_thread->user_esp = (unsigned int)(stack_base + (N * PAGE_SIZE) - 16); // Ajustar el puntero ESP
    new_thread->TID = global_TID++;

    // Inicializar el stack con los valores de retorno para la ejecución del hilo
    unsigned int *stack_top = (unsigned int *)new_thread->user_esp;
    //*(--stack_top) = (unsigned int)thread_exit;        // Dirección de la función de salida del hilo
    *(--stack_top) = (unsigned int)argument;               // Argumento que se pasa al hilo
    *(--stack_top) = (unsigned int)start_routine;     // Dirección de la función que ejecutará el hilo

    // Actualizar el puntero ESP del hilo
    new_thread->user_esp = (unsigned int)stack_top;

    list_add_tail(&new_thread->list_thread, &threads);

    return new_thread->TID;  // Retornar el ID del hilo creado
}



