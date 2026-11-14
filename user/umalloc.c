#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"
#include "kernel/riscv.h"

#ifndef PGSIZE
#define PGSIZE 4096
#endif
#ifndef SBRK_ERR
#define SBRK_ERR ((char *)-1)
#endif

#define DEBUG    1
#define NCLASS   7
#define MAXSMALL 2048

struct block {
  uint size;    // включая заголовок
  uint is_free; // 1 свободен, 0 занят
  struct block *next;
};

static struct block *small_free[NCLASS];
static struct block *large_free;
static char *heap_start;

static int
refill(int c)
{
  uint size = 32 << c;
  char *p = sbrk(PGSIZE);
  if (p == SBRK_ERR) {
    return -1;
  }
  struct block *b;
  uint x;
  for (x = 0; x < PGSIZE; x += size) {
    b = (struct block *)(p + x);
    b->size = size;
    b->is_free = 1;
    b->next = small_free[c];
    small_free[c] = b;
  }
  return 0;
}

static int
number_block(uint size)
{
  int c = 0;
  while ((32 << c) < size) {
    c++;
  }
  return c;
}

static struct block *
alloc_small(uint need)
{
  int c = number_block(need);
  struct block *b;

  if (small_free[c] == 0 && refill(c) < 0) {
    return 0;
  }

  b = small_free[c];
  small_free[c] = b->next;
  return b;
}

static struct block *
alloc_large(uint need)
{
  uint size = PGSIZE;
  char *p;
  while (size < need) {
    size += PGSIZE;
  }

  struct block *cur = large_free;
  struct block *prev = 0;

  while (cur) {
    if (cur->size >= size) {
      if (prev)
        prev->next = cur->next;
      else
        large_free = cur->next;
      return cur;
    }
    prev = cur;
    cur = cur->next;
  }

  struct block *b;
  p = sbrk(size);
  if (p == SBRK_ERR) {
    return 0;
  }
  b = (struct block *)p;
  b->size = size;
  return b;
}

static void
print_heap(void)
{
  char *end = sbrk(0);
  struct block *b;
  struct block *p;
  uint heap_size = end - heap_start;
  uint used_bytes = 0;
  uint used_blocks = 0;
  uint free_blocks = 0;
  uint bins[NCLASS];
  uint large = 0;
  int first = 1;
  int i;

  for (i = 0; i < NCLASS; i++) {
    bins[i] = 0;
    for (p = small_free[i]; p; p = p->next)
      bins[i]++;
  }
  for (p = large_free; p; p = p->next)
    large++;

  if (heap_size > 0) {
    b = (struct block *)heap_start;
    while ((char *)b < end) {
      if (b->is_free) {
        free_blocks++;
      } else {
        used_blocks++;
        used_bytes += b->size;
      }
      b = (struct block *)((char *)b + b->size);
    }
  }

  printf("  heap: brk=%p size=%d used=%d/%d (%d%%) blk=%d/%d", end,
         (int)heap_size, (int)used_bytes, (int)heap_size,
         heap_size ? (int)(used_bytes * 100 / heap_size) : 0,
         (int)used_blocks, (int)free_blocks);

  for (i = 0; i < NCLASS; i++) {
    if (bins[i]) {
      printf("%s %d:%d", first ? " free:" : "", 32 << i, (int)bins[i]);
      first = 0;
    }
  }
  if (large) {
    printf("%s large:%d", first ? " free:" : "", (int)large);
    first = 0;
  }

  printf("\n");
}

void
free(void *cleanBlock)
{
  struct block *b;
  int c;

  if (cleanBlock == 0)
    return;
  b = (struct block *)cleanBlock - 1; // остается только заголовок
  b->is_free = 1;

  if (b->size <= MAXSMALL) {
    c = number_block(b->size);
    b->next = small_free[c]; // в начало опредлееной корзину кидаем
    small_free[c] = b;
  } else if ((char *)b + b->size ==
             sbrk(0)) { // если это большой блок наверху кучи, то отдаем обратно
    sbrk(-(int)b->size);
  } else {
    b->next = large_free; // иначе ставим в начало больших блоков
    large_free = b;
  }

  if (DEBUG) {
    printf("free(%p)\n", cleanBlock);
    print_heap();
  }
}

void *
malloc(uint nbytes)
{
  uint need = nbytes + sizeof(struct block);
  struct block *b;

  if (nbytes == 0) {
    return 0;
  }

  if (heap_start == 0) {
    heap_start = sbrk(0);
  }

  if (need <= MAXSMALL)
    b = alloc_small(need);
  else
    b = alloc_large(need);
  if (b == 0)
    return 0;
  b->is_free = 0;

  if (DEBUG) {
    printf("malloc(%d) -> %p\n", nbytes, b + 1);
    print_heap();
  }

  return b + 1;
}