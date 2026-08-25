#include <string>
#include <print>
#include <ivl/command_line_argument_parsing/annotations>

struct [[=ivl::cmdline_parsing::class_basic]]
args {
  std::string foo;
  std::string bar;
};

int ivl_main(args& a) {
  std::println("foo: {}", a.foo);
  std::println("bar: {}", a.bar);
  return 4;
}
