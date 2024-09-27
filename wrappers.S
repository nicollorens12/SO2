#include <asm.h>
#include "entry.S" 

.globl keyboard_handler
keyboard_handler:
    SAVE_ALL            
    call keyboard_isr   
    RESTORE_ALL         
    iret                
