#include <iostream>

#include "fn-template.hpp"

template <typename T>
void print(const T& t) {
  std::cout << "Printing: " << t << std::endl;
}
