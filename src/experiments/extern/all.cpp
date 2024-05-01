#include <array>
#include <iostream>

std::array<int, 500000000> arr = []{
  std::array<int, 500000000> out;
  for (int i = 0; i < out.size(); ++i)
    out[i] += i + 42;
  return out;
 }();

void print_first(){
  std::cout << arr[0] << std::endl;
}

int main(){print_first();}
