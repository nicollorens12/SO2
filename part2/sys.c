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

#include <sem.h>

#include <libc.h>

#define LECTURA 0
#define ESCRIPTURA 1

extern struct list_head blocked;
extern struct list_head getKeyBlocked;
extern struct list_head key_blockedqueue;
extern struct list_head threads;

extern int pending_key;
extern struct sem_t sem_list[NUM_SEM];


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

int sys_gettid(){
  return current()->TID;
}

int global_PID=1000;
int global_TID=0;

int ret_from_fork()
{
  return 0;
}

__attribute__((optimize("O0"))) void* allocate_user_stack(int N, page_table_entry *process_PT) { //Como en el fork, hay que hacer una busqueda lineal por el directorio para ver donde ponerlo
  if(N > NUM_PAG_DATA) return NULL;
  int space = 0;
  

  int new_ph_pag, pag, i;
  for(int pag = 0; pag < TOTAL_PAGES-NUM_PAG_DATA*2-NUM_PAG_CODE-NUM_PAG_KERNEL; pag++) {
    if(is_page_used(process_PT, PAG_LOG_INIT_HEAP+pag) == 0){
      int pag_aux = pag;
      while(pag_aux - pag < N - 1){ // -1 por que se acaba de mirar que la de arriba esta vacia
        if(is_page_used(process_PT, PAG_LOG_INIT_HEAP+pag_aux) == 0){
          pag_aux++;
        }
        else{
          pag = pag_aux + 1;
          break;
        }
      }
      if(pag_aux - pag == N - 1){
        space = 1;
      }
    }
    if(space){
      for(i = pag; i < pag + N; i++){
        new_ph_pag=alloc_frame();
        if (new_ph_pag!=-1) /* One page allocated */
        {
          set_ss_pag(process_PT, PAG_LOG_INIT_HEAP+i, new_ph_pag);
        }
        else /* No more free pages left. Deallocate everything */
        {
          /* Deallocate allocated pages. Up to pag. */
          for (int j=pag; j<i; j++)
          {
            free_frame(get_frame(process_PT, PAG_LOG_INIT_HEAP+j)); //Faltava
            del_ss_pag(process_PT, PAG_LOG_INIT_HEAP+j);
          }
          
          /* Return error */
          return NULL; 
        }
      }
      return (void*)((PAG_LOG_INIT_HEAP+pag + N) << 12); // NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA + 
    }   
  }
  return NULL;
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

  // Cal revisar el HEAP sencer mirar si hi ha una pagina ocupada, i si esta ocupada copiarla
  for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA*2; pag< TOTAL_PAGES; pag++){
    if(is_page_used(parent_PT, pag)){
      int i = 1;
      while(is_page_used(parent_PT, pag + i)) ++i;
      void *space_base_pointer = allocate_user_stack(i, parent_PT);

      if(space_base_pointer == NULL) {
        for (int pag_aux=NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA*2; pag_aux<=pag; pag_aux++)
        {
          if(is_page_used(process_PT, pag_aux)){
            free_frame(get_frame(process_PT, pag_aux));
            del_ss_pag(process_PT, pag_aux);
          }
        }
        list_add_tail(lhcurrent, &freequeue);
            
        /* Return error */
        return -EAGAIN; 
      }
      
      int space_base_index = (unsigned int) space_base_pointer >> 12;
      int space_top_index = space_base_index - i;
      int j;

      for(j = space_top_index; j < pag + i; ++j){
        set_ss_pag(parent_PT, j, get_frame(process_PT,j));
        copy_data((void*)(pag<<12), (void*)((j)<<12), PAGE_SIZE);
        del_ss_pag(parent_PT, j);
      }

      pag += i - 1;
    }
  }


  /* Deny access to the child's memory space */
  set_cr3(get_DIR(current()));

  uchild->task.PID=++global_PID;
  uchild->task.TID=global_TID++;
  uchild->task.state=ST_READY;
  uchild->task.expiring_time = -1;
  INIT_LIST_HEAD(&uchild->task.threads);

  /* Prepare child stack */ // S'ha de fer? Potser no cal
  uchild->task.user_stack_base = NULL; 
  uchild->task.num_stack_pages = 0;

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
  // Si soc el propietari d'un semafor sys_semDestroy
  
  for(int i = 0; i < NUM_SEM; ++i){
    if(sem_list[i].TID == current()->TID)
      sys_semDestroy(&sem_list[i]);
  }

  int i;
  reduce_dir_reference(current());

  if(!list_empty(&current()->threads)){
    struct list_head *pos, *n;
    list_for_each_safe(pos, n, &current()->threads){
      struct task_struct *t = list_head_to_task_struct(pos);
      list_del(&t->list);
      if((&t->list_ordered)->next != 0){
        list_del(&t->list_ordered);
      }
      if((&t->list_thread)->next != 0){
        list_del(&t->list_thread);
      }
      t->PID = idle_task->PID;

      list_add_tail(&t->list_thread, &idle_task->threads);
    }
  }

  if(check_dir_references(current()) == 0){ //Si era el ultimo thread con este directorio, se libera
    page_table_entry *process_PT = get_PT(current());

    // Deallocate all the propietary physical pages
    for (i=0; i<NUM_PAG_DATA; i++)
    {
      free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
      del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
    }

    // Deallocate all heap (USER DYNAMIC STACKS)
    for (i = NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA; i < TOTAL_PAGES; ++i)
    {
      if (is_page_used(process_PT, i))
      {
        free_frame(get_frame(process_PT, i));
        del_ss_pag(process_PT, i);   
      }
    }
    

  }
  else{
    for(i = 0; i < current()->num_stack_pages; i++){
        free_frame(get_frame(get_PT(current()), ((int)current()->user_stack_base >> 12) - 1 + i));
        del_ss_pag(get_PT(current()), ((int)current()->user_stack_base >> 12) - 1 + i);
    }
  }
  /* Free task_struct */
  // list_add_tail(&(current()->list), &freequeue);

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
  if ( b != NULL && !access_ok(VERIFY_WRITE, b, 80 * 25 * 2) )
    return -EFAULT;  /* Bad address */
  
  if (b != NULL)
    dump_to_screen(b);
  else
  {
    // Moure el cursor a l'inici de la pantalla
    move_cursor(0, 0);  
    // Posem colors a negre
    change_color(0, 0);

    // Pintem totes les posicions buides
    for (int i = 0; i < 25; ++i)
      for (int j = 0; j < 80; ++j)
        printc(' ');
  }
  return 1;
}

struct sem_t* sys_semCreate(int initial_value)
{
    // Habra que buscar uno libre --> si no hay uno libre devolver error
    int i = get_sem_free_idx();
    if (i == -1)
      return NULL;
    
    struct sem_t *s = &sem_list[i];
    s->TID = current()->TID;
    s->count = initial_value;
    INIT_LIST_HEAD(&s->blocked);

    return s;
}

int sys_semWait(struct sem_t* s)
{
  // Addresa dins de la zona de memoria de la llista 
  if(s < &sem_list[0] || s> &sem_list[NUM_SEM]) return -ESEMINVADR;
  
  --(s->count);
  if (s->count < 0)
  {
    list_add_tail(&current()->list, &s->blocked);
    sched_next_rr();
    return current()->wake_reason; //Si se ha bloqueado
  }

  return 0; //Si no se ha bloqueado
}

int sys_semSignal(struct sem_t* s){
  //Comprabar rang adreces
  if(s < &sem_list[0] || s> &sem_list[NUM_SEM]) return -ESEMINVADR;


  ++(s->count);
  if (s->count <= 0)
  {
    if(list_empty(&s->blocked)) return -ESEMNOBLK;
    struct list_head *l = list_first( &(s->blocked) );
    list_del(l);
    struct task_struct *t = list_head_to_task_struct(l);
    t->wake_reason = SEM_SIG;
    list_add_tail(&t->list, &readyqueue);
    return 1; //Si ha desbloqueado algo
  }

  return 0; //Si no ha desbloqueado nada
}

int sys_semDestroy(struct sem_t* s)
{
  if(s < &sem_list[0] || s> &sem_list[NUM_SEM])
    return -ESEMINVADR;
  if (current()->TID != s->TID ) // Comprobar si el proceso en curso es el propietario del semaforo y por tanto, lo puede destruir
    return -ESEMNOPRP;

  s->TID = -1;
  s->count = 0;

  struct list_head *l = &s->blocked;

  struct list_head *pos, *n;
  list_for_each_safe(pos, n, l)
  {
      list_del(pos);
      struct task_struct *t = list_head_to_task_struct(l);
      t->wake_reason = SEM_DES;
      list_add_tail(&t->list, &readyqueue);
  }

  return 1;
}

// Si no hay semaforos libres devuelve -1
int get_sem_free_idx()
{
  for (int i = 0; i < NUM_SEM; ++i)
    if (sem_list[i].TID == -1)
      return i;

  return -1;
}

int sys_threadCreateWithStack(void (*wrapper_func)(void), void (*function)(void), int N, void *parameter )
{
    struct list_head *lhcurrent = NULL;
    union task_union *new_thread;

    if(function < (PAG_LOG_INIT_CODE << 12)|| function > ((PAG_LOG_INIT_CODE + NUM_PAG_CODE) << 12)) return -EINVFUNCADR;
    
    if (list_empty(&freequeue)) return -ENOMEM;

    lhcurrent = list_first(&freequeue);
    list_del(lhcurrent);
    new_thread = (union task_union*)list_head_to_task_struct(lhcurrent);
    copy_data((union task_union*) current(), new_thread, sizeof(union task_union)); 

    if(add_dir_reference(new_thread) == -1) return -ENOMEM;

    page_table_entry *process_PT = get_PT(current());

    void *stack_base = allocate_user_stack(N, process_PT); 
    if(stack_base == NULL){ 
      reduce_dir_reference(new_thread);
      list_add_tail(lhcurrent, &freequeue);
      return -ENOMEM;
    }

    unsigned int *stack_ptr = stack_base;

    stack_ptr -= 1;
    *(stack_ptr) = (unsigned int)parameter;
    stack_ptr -= 1;
    *(stack_ptr) = function; 
    stack_ptr -= 1;
    *stack_ptr = 0;

    new_thread->task.user_stack_base = stack_base;
    new_thread->task.num_stack_pages = N;
    new_thread->task.TID = global_TID++;
    new_thread->task.state = ST_READY;

    new_thread->stack[KERNEL_STACK_SIZE-2] = (unsigned int)stack_ptr;
    new_thread->stack[KERNEL_STACK_SIZE-5] = (unsigned int)wrapper_func;

    int register_ebp = (int) get_ebp();
    register_ebp = (register_ebp - (int)current()) + (int)(new_thread);
    new_thread->task.register_esp = register_ebp + sizeof(DWord);

    DWord temp_ebp = *(DWord *)register_ebp;
    new_thread->task.register_esp -= sizeof(DWord);
    *(DWord *)(new_thread->task.register_esp) = (DWord)&ret_from_fork;
    new_thread->task.register_esp -= sizeof(DWord);
    *(DWord *)(new_thread->task.register_esp) = temp_ebp;

    INIT_LIST_HEAD(&new_thread->task.threads);

    list_add_tail(&new_thread->task.list_thread, &current()->threads);
    list_add_tail(&new_thread->task.list, &readyqueue);

    return new_thread->task.TID;
}


char* sys_memRegGet(int num_pages) {
  if(num_pages > (TOTAL_PAGES-NUM_PAG_DATA*2-NUM_PAG_CODE-NUM_PAG_KERNEL)) return -1;
  int space = 0;
  page_table_entry * process_PT = get_PT(current());

  int new_ph_pag, pag, i;
  for(int pag = 0; pag < TOTAL_PAGES-NUM_PAG_DATA*2-NUM_PAG_CODE-NUM_PAG_KERNEL; pag++) {
    if(is_page_used(process_PT ,PAG_LOG_INIT_HEAP+pag) == 0){
      int pag_aux = pag;
      while(pag_aux - pag< num_pages){
        if(is_page_used(process_PT, PAG_LOG_INIT_HEAP+pag_aux) == 0){
          pag_aux++;
        }
        else{
          pag = pag_aux + 1;
          break;
        }
      }
      if(pag_aux - pag == num_pages){
        space = 1;
      }
    }
    if(space){
      for(i = pag; i < pag + num_pages + 1; i++){
        new_ph_pag=alloc_frame();
        if (new_ph_pag!=-1) 
        {
          set_ss_pag(process_PT, PAG_LOG_INIT_HEAP+i, new_ph_pag);
          if(i == pag) {
            set_page_rw4system(process_PT, PAG_LOG_INIT_HEAP+i); // La primera pagina es la especial y marcamos con esta funcion que es de sistema
          }
        }
        else /* No more free pages left. Deallocate everything */
        {
          /* Deallocate allocated pages. Up to pag. */
          for (int j=pag; j<i; j++)
          {
            free_frame(get_frame(process_PT, PAG_LOG_INIT_HEAP+i)); //Faltava
            del_ss_pag(process_PT, PAG_LOG_INIT_HEAP+j);
          }
          
          /* Return error */
          return -EAGAIN; 
        }
      }
      // Informacion guardada en la primera pagina de la region (la reservada para sistema)
      char *ret = (char *)((PAG_LOG_INIT_HEAP + pag) << 12); // Dirección lógica inicial de la región
      *(unsigned int *)ret = (unsigned int)((PAG_LOG_INIT_HEAP + pag + 1) << 12); // Guardar la dirección "propietaria" de la región
      *(ret + sizeof(unsigned int)) = (char)num_pages; // Guardar el número de páginas de la región

      return (char*)((PAG_LOG_INIT_HEAP+pag + 1) << 12); // It returns the initial logical address assigned to the region
    }   
  }
  return NULL;
}


int sys_memRegDel(char* m){ // Busca una zona de tipo sys_memRegGet, es decir, que al final de la region tenga un pagina extra con present = 1 pero rw = 0
  if ((unsigned long)m % PAGE_SIZE != 0) return -1; // Check if m is aligned with PAGE_SIZE
  int pag = ((int)m) >> 12;
  if(pag < NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA*2 || pag >= TOTAL_PAGES) return -1;
  if(*(char**)((pag - 1) << 12) != *m) return -1;

  page_table_entry * process_PT = get_PT(current());
  
  int num_pages = *(char*)((pag - 1) << 12 + 1);

  for(int i = pag - 1; i < pag + num_pages; i++){
    free_frame(get_frame(process_PT, i));
    del_ss_pag(process_PT, i);
  }

  set_cr3(get_DIR(current()));

  return 1;
}
