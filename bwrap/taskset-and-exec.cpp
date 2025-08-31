#include <cassert>
#include <cstdlib>
#include <sched.h>
#include <seccomp.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) {
  assert(argc >= 3);

  { // taskset
    auto      core = atoi(argv[1]);
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(core, &mask);
    if (-1 == sched_setaffinity(0, sizeof(mask), &mask)) {
      perror("sched_setaffinity");
      return 1;
    }
  }

  { // drop syscall permissions
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
    if (ctx == NULL) {
      perror("seccomp_init");
      return 1;
    }

    if (0 > seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(sched_setaffinity), 0)) {
      perror("seccomp_rule_add(sched_setaffinity)");
      return 1;
    }
    if (0 > seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(sched_getaffinity), 0)) {
      perror("seccomp_rule_add(sched_getaffinity)");
      return 1;
    }

    if (0 > seccomp_load(ctx)) {
      perror("seccomp_load");
      return 1;
    }
  }

  execv(argv[2], argv + 2);
}
