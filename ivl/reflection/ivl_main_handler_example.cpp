#include <print>
#include <optional>
#include <string>

struct args {
  int foo;
  int bar;
  std::optional<std::string> str;
};

int ivl_main(args& args) {
  std::println("foo: {}", args.foo);
  std::println("bar: {}", args.bar);
  return 0;
}
