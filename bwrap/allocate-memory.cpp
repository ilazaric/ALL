#include <cassert>
#include <cstdlib>
#include <iostream>
#include <new>
#include <unistd.h>

// int main(int argc, char** argv){
//   // return 0;
//   assert(argc == 2);
//   auto size = atoll(argv[1]);
//   auto memory = new char[size];
//   for (long long i = 0; i < size; ++i)
//     memory[i] = 42;
//   // sleep(5);
//   delete[] memory;
//   return 0;
// }

int main() {
  size_t size;
  std::cin >> size;
  auto memory = new char[size];
  for (long long i = 0; i < size; ++i)
    memory[i] = 42;
  delete[] memory;
  std::cout << "Done" << std::endl;
  sleep(5);
  std::cout << "Done" << std::endl;
  return 0;
}

// int main(){}
