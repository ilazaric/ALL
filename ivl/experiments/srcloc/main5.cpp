#include <cassert>
#include <source_location>

consteval int strlen(const char* a) {
  int len = 0;
  while (a[len]) ++len;
  return len;
}

consteval bool check(std::source_location loc) {
  assert(loc.line() == 16);
  assert(strlen(loc.function_name()) == 0);
  return true;
}

template<auto S = check(std::source_location::current())>
void fn() {}

int main() { fn(); }
