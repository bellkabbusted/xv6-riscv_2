#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
check(char *name, int ok)
{
  printf("%s: %s\n", name, ok ? "OK" : "FAIL");
}

int
main(void)
{
  char buf[4];
  struct stat st;
  int m = mutex();

  check("mutex() >= 0", m >= 0);
  check("unlock without lock -> <0", mutex_unlock(m) < 0);
  check("lock -> 0", mutex_lock(m) == 0);
  check("second lock -> <0", mutex_lock(m) < 0);
  check("read -> <0", read(m, buf, 4) < 0);
  check("write -> <0", write(m, buf, 4) < 0);
  check("fstat -> <0", fstat(m, &st) < 0);

  int pid = fork();
  if (pid == 0) {
    check("unlock of foreign lock -> <0", mutex_unlock(m) < 0);
    exit(0);
  }
  wait(0);
  check("unlock -> 0", mutex_unlock(m) == 0);

  mutex_lock(m);
  pid = fork();
  if (pid == 0) {
    pause(10);
    check("child locks after parent close", mutex_lock(m) == 0);
    exit(0);
  }
  check("close of held mutex -> 0", close(m) == 0);
  wait(0);
  check("lock on closed fd -> <0", mutex_lock(m) < 0);
  exit(0);
}