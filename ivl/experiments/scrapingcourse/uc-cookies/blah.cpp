#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
  auto pid = fork();
  if (pid == 0) {
    open("/tmp", O_RDONLY, 0);
  } else if (pid > 0) {
    open("/", O_RDONLY, 0);
    waitpid(pid, nullptr, 0);
  }
}
