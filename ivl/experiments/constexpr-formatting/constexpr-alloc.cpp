#include <new>
#include <string_view>

consteval {
  int x = 123;
  auto ptr = new char[x];
  auto sv = std::string_view(ptr, x);
  auto ptr2 = sv.data();
  delete[] ptr2;
}
