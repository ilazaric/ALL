#include <iostream>

// Input for unshare_example.

int main() {
  std::cout << "hello world" << std::endl;
  volatile int v = 123;
  std::cout << "stack: " << (void*)&v << std::endl;
  return v;
}
