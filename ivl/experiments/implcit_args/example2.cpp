#include <ivl/command_line_argument_parsing/implicit>
#include <ivl/command_line_argument_parsing/passthrough>

#include <print>

// IVL add_compiler_flags("-Wno-non-template-friend -Wsfinae-incomplete=0")

int ivl_main(ivl::cmdline_parsing::implicit, ivl::cmdline_parsing::passthrough pass) {
  using namespace ivl::cmdline_parsing::implicit_functions;
  std::println("remaining args: {::?}", pass.data);
  std::println("foo: {}", implicit_flag("foo"));
  std::println("bar: {}", implicit_flag("bar"));
  std::println("number: {}", implicit_value("number", -42));
  return 0;
}
