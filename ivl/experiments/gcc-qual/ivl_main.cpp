#include <string>
#include <print>

struct args {
  std::string foo;
  std::string bar;
};

int ivl_main(args& a) {
  std::println("foo: {}", a.foo);
  std::println("bar: {}", a.bar);
  return 4;
}
