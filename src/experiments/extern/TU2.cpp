#include <array>

std::array<int, 500000000> arr = []{
  // launch multiple threads here if you want
  std::array<int, 500000000> out;
  for (int i = 0; i < out.size(); ++i)
    out[i] += i + 42;
  return out;
 }();
