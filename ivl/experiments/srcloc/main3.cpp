#include <cstdio>
#include <source_location>

template<int N>
struct S {
  static constexpr int value = N;
};

template<typename T = S<std::source_location::current().line()>>
void fn() {
  printf("line: %d\n", (int)T::value);
}

int main() { fn(); }
