#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

int main() {
  auto pid = fork();
  if (pid < 0) exit(1);
  if (pid == 0) exit(2);
  int wstatus = -1;
  wait4(pid, &wstatus, 0, NULL);
  if (WIFEXITED(wstatus) && WEXITSTATUS(wstatus) == 2) exit(0);
  else {
    fprintf(stderr, "wstatus: %d\n", wstatus);
    exit(3);
  }
}
