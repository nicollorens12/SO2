#include <libc.h>
#include <screen_helper.h>
#include <dangerous_dave.h>


int __attribute__ ((__section__(".text.main")))
  main(void)
{
  game_loop();
  
  while(1) { 
    
  }
}