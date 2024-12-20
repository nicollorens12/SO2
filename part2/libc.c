/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

int errno;
int REGS[7]; // Space to save REGISTERS

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

void perror()
{
  char buffer[256];

  itoa(errno, buffer);

  write(1, buffer, strlen(buffer));
}

// En libc.c
void itodec(int a, char *b)
{
  int i, i1, digit;
  char c;

  if (a==0) { b[0]='0'; b[1]=0; return ;}

  i=0;
  while (a>0)
  {
    digit=(a%10);
    a=a/10;

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

void* wrapper_func(void (*function)(void* arg), void* parameter)
{
  (*function)(parameter);
  exit();
}



Slab slab_create(int n_pages, int block_size) {
    Slab slab;
    slab.num_blocks = 0;

    if (block_size <= 0) return slab;

    int total_block_size = block_size + sizeof(struct list_head);
    int num_blocks = (0x1000)*n_pages / total_block_size; // Bloques enteros que caben en la página
    if (num_blocks <= 0) return slab; 

    char *start = memRegGet(n_pages);
    if (!start) return slab;

    slab.start = start;
    slab.block_size = block_size;
    slab.num_blocks = num_blocks;
    slab.used_blocks = 0;

    INIT_LIST_HEAD(&slab.free_list);

    // Crear la lista de bloques libres dentro de la página
    char *block = start;
    for (int i = 0; i < num_blocks; i++) {
        struct list_head *entry = (struct list_head *)block;
        list_add_tail(entry, &slab.free_list);
        block += total_block_size;
    }

    return slab;
}

// Asigna un bloque del slab y lo saca de la lista de libres
void *slab_alloc(Slab *slab) {
    if (!slab || list_empty(&slab->free_list)) return NULL;

    struct list_head *block = list_first(&slab->free_list);
    list_del(block);
    slab->used_blocks++;

    return (void *)block;
}


// Libera un bloque y lo devuelve a la lista de libres
void slab_free(Slab *slab, void *ptr) {
    if (!slab || !ptr) return;

    struct list_head *block_list_head = (struct list_head *)ptr;

    list_add_tail(block_list_head, &slab->free_list);

    slab->used_blocks--;
}

// Destruye el slab y libera toda la memoria
void slab_destroy(Slab *slab) {
    if (!slab) return;

    memRegDel(slab->start);
}