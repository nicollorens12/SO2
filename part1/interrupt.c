/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>

#include <zeos_interrupt.h>

//#define EOI asm volatile("movb $0x20, %al; outb %al, $0x20")

Gate idt[IDT_ENTRIES];
Register    idtR;

extern int zeos_ticks;
void syscall_handler_sysenter();
void writeMSR(unsigned long msr, unsigned long val);

void new_page_fault_handler();
void keyboard_handler();
void clock_handler();
void system_call_handler();

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



void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
  set_handlers();
  setInterruptHandler(14, new_page_fault_handler, 3);
  setInterruptHandler(33, keyboard_handler, 0);
  setInterruptHandler(32, clock_handler, 0);
  
  setTrapHandler(0x80, system_call_handler, 3);

  writeMSR(0x174, __KERNEL_CS);
  writeMSR(0x175, INITIAL_ESP);
  writeMSR(0x176, (unsigned long)syscall_handler_sysenter);

  set_idt_reg(&idtR);
}



void keyboard_routine() {
    // Leer el código de la tecla del puerto 0x60 (puerto del teclado)
    unsigned char scancode = inb(0x60);
    // Si el bit más significativo no está en 1, significa que es una tecla presionada (no liberada)
    if (!(scancode & 0x80)) {
        char key = char_map[scancode];
        int errorSameProcess = 0;
        printc(key);
        if(key == 'p'){ //Swap
          struct task_struct* pcb = current();
          struct task_struct* next_pcb = list_head_to_task_struct(list_first(&readyqueue));
          if (next_pcb->PID == pcb->PID) {
            errorSameProcess = 1;
          } else {
            inner_task_switch((union task_union*)next_pcb);

          }
        }

        if(errorSameProcess){
          printk("Error: Can't change to the same process\n");
        }
        else{
          if(current()->PID != 0){
            printk("Process changed ");
            char buff[256];
            itohex(current()->PID, buff);
            printk(buff);
            printk("\n");
          }
        }
        printc(key);
    }

}


void clock_routine(){
  zeos_ticks++;
  zeos_show_clock();

  schedule();
}



// Convert int into hex 0x format string --> MOURE'L A LIBC.H
void itohex(int a, char *b);

void itohex(int a, char *b)
{
  int i, i1, digit;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    digit=(a%16);
    a=a/16;

    if (digit < 10)
      b[i] = digit + '0';
    else
      b[i] = 'A' + digit - 10;

    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

void new_page_fault_routine(int address) {
  printk("\nProcess generates a PAGE FAULT exception at EIP: 0x");

  char buff[256];
  itohex(address, buff);
  printk(buff);

  printk("\n");

  while(1);
}