#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>

#include <unistd.h>

// void use(){
//   int fd = 0;
//   close(fd);
// }

int main() {
  int (*ptr)(int) = close;
  std::cout << std::hex << reinterpret_cast<std::uintptr_t>(ptr) << std::endl;
}
