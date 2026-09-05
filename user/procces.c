#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  int mode_b = (argc > 1 && argv[1][0] == 'b');

  int pid = fork();

  if (pid < 0) {
    printf("fork failed\n");
    exit(1);
  }

  if (pid == 0) { //child
    int pause_sec = 10; 
    printf("child %d: sleeping for %d sec\n", getpid(), pause_sec);
    pause(pause_sec * 10); 
    exit(1);              
  }

  // parent
  printf("parent pid: %d, child pid: %d\n", getpid(), pid);

  if (mode_b) {
    // kill child
    printf("parent: sending kill to %d\n", pid);
    kill(pid);
  }

  int status;
  int finished_pid = wait(&status);

  printf("child %d finished, exit code: %d\n", finished_pid, status);
  exit(0);
}