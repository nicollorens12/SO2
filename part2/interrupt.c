/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>

#include <sched.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;


extern struct list_head key_blockedqueue; //FIFO List mantain order of getKey call
extern struct list_head getKeyBlocked;  //Ordered list by expring time
extern int pending_key;

// Circular Buffer Definition
void init_circular_buffer(struct CircularBuffer *cb){
  cb->head = 0;
  cb->tail = 0;
  cb->chars_written = 0; 
}

void add_element_cb(struct CircularBuffer *cb, char e){
  cb->buffer[cb->tail] = e; 
  cb->chars_written++;
  if(cb->tail == sizeof(cb->buffer) - 1){
    cb->tail = 0;
  }
  else cb->tail++;
}

int read_element_cb(struct CircularBuffer *cb, char *c){
  if (cb->chars_written != 0){
    *c = cb->buffer[cb->head];
    if(cb->head == sizeof(cb->buffer) - 1){
      cb->head = 0;
    }
    else cb->head++;
    cb->chars_written--;
    return 1;
  }
  return 0;
  
}

int is_empty_cb(struct CircularBuffer *cb){
  return cb->chars_written == 0;
}

int get_chars_pending_cb(struct CircularBuffer *cb){
  return cb->chars_written;
}

struct CircularBuffer circular_buffer;

char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','�','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','�',
  '\0','�','\0','�','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

int zeos_ticks = 0;

void check_getKey_timeouts(){
  if(!list_empty(&getKeyBlocked)){
    
    struct list_head *pos, *n;
    list_for_each_safe(pos, n, &getKeyBlocked){
      struct task_struct *task = list_entry(pos, struct task_struct, list_ordered);

      if(task->expiring_time <= zeos_ticks){
        list_del(&task->list_ordered);
        //Hay que eliminar de la key_blockedqueue tambien
        update_process_state_rr(&task, &readyqueue);
      }
      else break;

    }
    
  }
}
 
void clock_routine()
{
  zeos_show_clock();
  zeos_ticks ++;
  check_getKey_timeouts();
  schedule();
}

void keyboard_routine()
{
  unsigned char c = inb(0x60);
  
  if (c&0x80){
    printc_xy(0, 0, char_map[c&0x7f]);
    add_element_cb(&circular_buffer, char_map[c&0x7f]);

    if (!list_empty(&key_blockedqueue)) {
            struct list_head *first = list_first(&key_blockedqueue);
            
            struct task_struct *task = list_entry(first, struct task_struct, list);
            list_del(&task->list_ordered);
            update_process_state_rr(task, &readyqueue);
            pending_key++; //quizas se puede quitar y usar directamente chars_written del cb
        }

  } 
}

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void clock_handler();
void keyboard_handler();
void system_call_handler();

void setMSR(unsigned long msr_number, unsigned long high, unsigned long low);

void setSysenter()
{
  setMSR(0x174, 0, __KERNEL_CS);
  setMSR(0x175, 0, INITIAL_ESP);
  setMSR(0x176, 0, (unsigned long)system_call_handler);
}

void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */
  setInterruptHandler(32, clock_handler, 0);
  setInterruptHandler(33, keyboard_handler, 0);

  setSysenter();

  set_idt_reg(&idtR);
}

