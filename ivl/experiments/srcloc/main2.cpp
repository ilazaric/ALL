#include <source_location>
#include <cstdio>

template<auto loc = [] (std::source_location x = std::source_location::current()) {
  return x.line();
}() >
void fn() {
    printf("line: %d\n", (int)loc);
}

int main() {
  fn();
}
