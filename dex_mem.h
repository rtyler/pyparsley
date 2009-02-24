#ifndef DEX_MEM_H_INCLUDED
#define DEX_MEM_H_INCLUDED

#define obstack_chunk_alloc malloc
#define obstack_chunk_free free

#include "obstack.h"

void dex_collect();
void * dex_alloc(int size);


#endif