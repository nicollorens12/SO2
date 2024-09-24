#include <asm.h>
#include "entry.S" 

.extern keyboard_isr

.globl keyboard_handler
keyboard_handler:
    SAVE_ALL            // Guardar los registros del contexto actual
    call keyboard_isr   // Llamar a la rutina de servicio del teclado en C
    RESTORE_ALL         // Restaurar los registros
    iret                // Regresar de la interrupción
