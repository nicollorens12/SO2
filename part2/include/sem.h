#ifndef __H_SEM_SYNCHRO__
#define __H_SEM_SYNCHRO__

#include <list.h>

struct sem_t
{
	int count;
	struct list_head blocked;
};

#endif __H_SEM_SYNCHRO__