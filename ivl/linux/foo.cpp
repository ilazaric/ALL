#include <signal.h>
#include <sys/prctl.h>
#include <thread>

// IVL add_compiler_flags("-static")

int main() {
  std::jthread{[] {
    auto pid = fork();
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL);
      while (true);
    } else {
      sleep(1);
    }
  }};
  sleep(3);
  return 1;
}
