#define __USE_GNU
#include <assert.h>
#include <fstream>
#include <iostream>
#include <source_location>
#include <sys/fsuid.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

void check(std::source_location loc = std::source_location::current()) {
  struct stat statbuf;
  assert_perror(lstat("/proc/self/uid_map", &statbuf));
  std::cout << "line=" << loc.line() << " pid=" << getpid() << " file_uid=" << statbuf.st_uid << std::endl;
}

void status(std::source_location loc = std::source_location::current()) {
  std::cout << "line=" << loc.line() << " pid=" << getpid() << " getuid=" << getuid() << " ";
  std::ifstream fin("/proc/self/status");
  std::string line;
  while (std::getline(fin, line))
    if (line.contains("Uid")) std::cout << line << std::endl;
}

int main(int argc, char* argv[]) {
  check();
  if (getuid() != 0) return 0;

  status();
  assert_perror(setuid(1000));
  status();

  check();
  auto pid = fork();
  if (pid == 0) {
    check();
    status();
    execve(argv[0], argv, nullptr);
  }
  waitpid(pid, nullptr, 0);
}
