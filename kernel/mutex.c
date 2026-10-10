#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"
#include "fs.h"
#include "file.h"

// create mutex as open file(pipealloc)
// Return 0/-1
int
mutexalloc(struct file **f)
{
  struct sleeplock *m;

  *f = 0;
  if((*f = filealloc()) == 0)
    return -1;

  if((m = kalloc()) == 0){
    fileclose(*f);
    *f = 0;
    return -1;
  }
  printk("mutex kalloc: %p pid %d\n", m, myproc()->pid);

  initsleeplock(m, "umutex");       // created free

  (*f)->type = FD_MUTEX;
  (*f)->readable = 0;               // fileread/filewrite return -1
  (*f)->writable = 0;
  (*f)->mutex = m;

  printk("mutexalloc: mutex %p pid %d\n", m, myproc()->pid);
  return 0;
}

// delete mutex. call from fileclose when there are no open descriptors
void
mutexclose(struct sleeplock *m)
{
  printk("mutexclose: mutex %p pid %d\n", m, myproc()->pid);
  kfree(m);
  printk("mutex kfree: %p pid %d\n", m, myproc()->pid);
}