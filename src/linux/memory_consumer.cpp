#include <vector>
#include <cstdlib>

// IVL add_compiler_flags("-static -flto")

int main(int argc, char** argv){
  size_t mem = atoll(argv[1]);
  char* data = (char*)malloc(mem);
  for (size_t i = 0; i < mem; ++i) data[i] = 1;
  int ret = 0;
  for (size_t i = 0; i < mem; ++i) ret += data[i]-1;
  return ret;
}
