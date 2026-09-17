#include <ivl/command_line_argument_parsing/annotations>
#include <optional>
#include <ivl/format>
#include <string>

struct[[= ivl::cmdline_parsing::class_basic]] args {
  int foo;
  int bar;
  // std::optional<std::string> str;
};

int ivl_main(args& args) {
  ivl::fmt::println("foo: {}", args.foo);
  ivl::fmt::println("bar: {}", args.bar);
  return 0;
}
