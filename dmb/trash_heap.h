#ifndef TRASH_HEAP
#define TRASH_HEAP
#include <sys/types.h>

void *get_trash(size_t n);

void free_trash(void *p);
#endif
