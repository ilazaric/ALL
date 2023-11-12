#include <chrono>
#include <iostream>

#include "big-family.hpp"

std::mt19937 gen(101);

int main(){
  int repcount = 0;
  double sizesum = 0;
  for (auto start = std::chrono::high_resolution_clock::now();
       std::chrono::high_resolution_clock::now() - start < std::chrono::seconds(5);
       ++repcount){
    auto bla = BigFamily::random(gen, 10, 10, 0.3);
    sizesum += (double)bla.family.size();
  }

  std::cout << "repcount: " << repcount << std::endl;
  std::cout << "expected size: " << sizesum / repcount << std::endl;
}
