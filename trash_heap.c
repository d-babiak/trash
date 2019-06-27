#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define MAGIC_TRASH 0x0123456789abcdef
#define PAGE_SIZE 4096

#define b_end(b) (b + sizeof(block_t) + b->size)

typedef struct block {
  struct block *next;
  size_t size;   // should size include the metadata?
} block_t;

static block_t *trash_heap = NULL; // in bss, zerod to start?

block_t *B_init(size_t n) {
  size_t n_pages = 1;
  
  while ((sizeof(block_t) + n) > PAGE_SIZE * n_pages)
    n_pages++;

  block_t *b = malloc(PAGE_SIZE * n_pages); // lol
  b->next = NULL;
  b->size = PAGE_SIZE * n_pages;

  return b;
}

block_t *split(block_t *b, size_t n) {
  assert((b_end(b) - b) >= (2*sizeof(block_t) + sizeof(void *) + n));

  block_t *new_block = (block_t *)(b_end(b) - (sizeof(block_t) + n));
  new_block->next    = (block_t * ) MAGIC_TRASH;
  new_block->size    = n;

  b->size -= (sizeof(block_t) + n);

  return new_block;
}


void *get_trash(size_t n) {
  if (trash_heap == NULL)
    trash_heap = B_init(n);


  // there is the case of a allocating the first node
  // and the case of allocating an internal node
  // do we always want to split?
  // not always
  block_t *b = trash_heap;

  if (b->size >= (2*sizeof(block_t) + sizeof(void *) + n)) {
    block_t *new_block = split(b, n);
    printf("ALLOC %d %p\n", __LINE__, new_block);
    return ((void*) new_block) + sizeof(block_t);
  }
  else if (b->size >= n) {
    trash_heap = b->next;
    b->next = (block_t *) MAGIC_TRASH;
    printf("ALLOC %d %p\n", __LINE__, b);
    return ((void *) b) + sizeof(block_t);
  }


  for (; b->next && b->next->size < n; b = b->next)
    ;

  assert(b->next && b->next->size >= n); // todo - mmap if no block big enough


  if (b->next->size >= (2*sizeof(block_t) + sizeof(void *) + n)) {
    block_t *new_block = split(b->next, n);
    printf("ALLOC %d %p\n", __LINE__, new_block);
    return ((void *) new_block) + sizeof(block_t);
  }
  else if (b->next->size >= n) {
    block_t *allocd = b->next;
    b->next = b->next->next;
    allocd->next = (block_t *) MAGIC_TRASH;
    printf("ALLOC %d %p\n", __LINE__, allocd);
    return ((void *) allocd) + sizeof(block_t);
  }
  else {
    printf("UNPOSSIBLE %d\n", __LINE__);
    exit(__LINE__);
  }
}

void free_trash(void *p) {
  block_t *to_free = (block_t *)(p - sizeof(block_t));
  if (to_free <= 0) {
    printf("UNPOSSIBLE %d\n", __LINE__);
    exit(__LINE__);
  }

  if (to_free->next != (block_t *)MAGIC_TRASH) {
    printf("(WTF): next: %p, %lu\n", to_free->next, to_free->size);
    exit(__LINE__);
  }

  if (trash_heap == NULL || to_free < trash_heap) {
    to_free->next = trash_heap;
    trash_heap    = to_free;
    return;
  }

  block_t *b;
  for (b = trash_heap; b->next && b->next < to_free; b = b->next)
    ;

  to_free->next = b->next;
  b->next       = to_free;

  // TODO - coalesce
}

void print_int(char c, int *x) {
  printf("%c: %p, *%c: %d\n", c, x, c, *x);
}

int main(int argc, char *argv[]) {
  int *x = (int *) get_trash(sizeof(int));
  print_int('x', x);
  *x = 42;



  int *y = (int *) get_trash(sizeof(int));
  *y = 43;
  print_int('x', x);
  print_int('y', y);

  free_trash(y);
  print_int('x', x);
  free_trash(x);
  printf("\n");
}
