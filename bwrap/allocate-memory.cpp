#include <new>
#include <cassert>
#include <cstdlib>
#include <unistd.h>

int main(int argc, char** argv){
  assert(argc == 2);
  auto size = atoll(argv[1]);
  auto memory = new char[size];
  for (long long i = 0; i < size; ++i)
    memory[i] = 42;
  sleep(5);
  delete[] memory;
  return 0;
}
