#include <source_location>
#include <cstdio>

template<auto loc = [] (int x = std::source_location::current().line()) {
    return x;
}() >
void fn() {
    printf("line: %d\n", (int)loc);
}

int main() {
  fn();
}
