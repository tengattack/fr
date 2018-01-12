
#ifndef _LIB_TC_MALLOC_H_
#define _LIB_TC_MALLOC_H_ 1

#include <malloc.h>

#define tc_malloc malloc
#define tc_free free
#define tc_malloc_size _msize

#define tc_realloc realloc
#define tc_valloc malloc

#endif