#include <cstdio>
#include <source_location>

template<int N>
struct S {
  static constexpr int value = N;
};

template<typename T = decltype([](std::source_location loc = std::source_location::current()) { return loc.line(); }), auto V = T{}()>
void fn() {
  printf("line: %d\n", (int)V);
}

int main() { fn(); }
