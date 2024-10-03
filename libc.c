/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>
#include <errno.h>

int errno;

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
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

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

void perror(void) 
{
  /* 
  NO ME ESTA DEJANDO IMPORTAR #include <string.h>
  DE MOMENTO LO PONGO 'FEO'
  > Supongo que habria que implementarlo a mano

  // Como es un array --> En C se usar strcpy para assignar strings
  //                      char* strcpy(char* destination, const char* source);
  char msg[256];

  switch(errno) {
    case ENOSYS:
      strcpy(msg, "Function not implemented");
      break;
    default:
      strcpy(msg, "Error not described");
      break;
  }
  write(1, msg, strlen(msg));
  */

  switch(errno) {
    char* msg;
    case ENOSYS:
      msg = "Function not implemented";
      write(1, msg, strlen(msg));
      break;
    default:
      msg = "Error not described";
      write(1, msg, strlen(msg));
      break;
  }
}