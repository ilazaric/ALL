#include <array>
#include <iostream>

extern std::array<int, 500000000> arr;

void print_first() { std::cout << arr[0] << std::endl; }
