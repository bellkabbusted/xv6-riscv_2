#include "kernel/types.h"
#include "user/user.h"

void
print_args(int argc, char *argv[], int first, int m, int sync)
{
  for (int i = first; i < argc; i++) {
    for (char *c = argv[i]; *c; c++) {
      if (sync)
        mutex_lock(m);
      printf("%d: arg %d, char '%c'\n", getpid(), i, *c);
      if (sync)
        mutex_unlock(m);
    }
  }
}

int
main(int argc, char *argv[])
{
  int sync = 0, first = 1, m = -1;

  if (argc > 1 && strcmp(argv[1], "-s") == 0) {
    sync = 1;
    first = 2;
    m = mutex();
    if (m < 0) {
      printf("mutex() failed\n");
      exit(1);
    }
  }

  int pid = fork();
  if (pid < 0) {
    printf("fork failed\n");
    exit(1);
  }
  print_args(argc, argv, first, m, sync);
  if (pid > 0)
    wait(0);
  if (sync)
    close(m);
  exit(0);
}