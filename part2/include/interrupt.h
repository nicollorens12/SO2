/*
 * interrupt.h - Definició de les diferents rutines de tractament d'exepcions
 */

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <types.h>

#define IDT_ENTRIES 256

extern Gate idt[IDT_ENTRIES];
extern Register idtR;

extern struct CircularBuffer circular_buffer;

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL);
void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL);

void setIdt();

struct CircularBuffer{
  char buffer[10];
  int chars_written;
  int head;
  int tail;
};

void init_circular_buffer(struct CircularBuffer *cb);

void add_element_cb(struct CircularBuffer *cb, char e);

char read_element_cb(struct CircularBuffer *cb, char *c);

#endif  /* __INTERRUPT_H__ */
