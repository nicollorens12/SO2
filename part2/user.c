#include <libc.h>
#include <screen_helper.h>
#include <dangerous_dave.h>


int __attribute__ ((__section__(".text.main")))
  main(void)
{
  game_loop();

    //Slab *slab = slab_create(64);
    //if (!slab) {
    //    // Error al crear el slab
    //    return;
    //}
//
    //// Asignar 3 objetos de 64 bytes
    //void *obj1 = slab_alloc(slab);
    //void *obj2 = slab_alloc(slab);
    //void *obj3 = slab_alloc(slab);
//
    //// Usar los objetos sin usar copy_data
    //char *src = "Hello World!";
    //char *dst = (char *)obj1;
    //for (int i = 0; i < 12; i++) {
    //    dst[i] = src[i];
    //}
//
    //write(1, obj1, 12);
    //write(1, "\n", 1);
    //write(1, dst, 12);
//
    //// Liberar los objetos
    //slab_free(slab, obj1);
    //slab_free(slab, obj2);
    //slab_free(slab, obj3);
//
    //// Destruir el slab
    //slab_destroy(slab);
  
  while(1) { 
    
  }
}