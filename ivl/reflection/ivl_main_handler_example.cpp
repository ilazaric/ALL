#include <ivl/command_line_argument_parsing/annotations>
#include <optional>
#include <print>
#include <string>

struct[[= ivl::cmdline_parsing::class_basic]] args {
  int foo;
  int bar;
  // std::optional<std::string> str;
};

int ivl_main(args& args) {
  std::println("foo: {}", args.foo);
  std::println("bar: {}", args.bar);
  return 0;
}
