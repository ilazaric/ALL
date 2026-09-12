#include "implicit"

#include <print>

// IVL add_compiler_flags("-Wno-non-template-friend -Wsfinae-incomplete=0")

int main2(std::span<const char* const> args) {
  std::println("remaining args: {::?}", args);
  std::println("foo: {}", implicit_flag("foo"));
  std::println("bar: {}", implicit_flag("bar"));
  std::println("number: {}", implicit_value("number", -42));
  return 0;
}

int main(int argc, const char* const* argv) try {
  std::span<const char* const> args(argv + !!argc, argv + argc);
  implicit_parse(args);
  return main2(args);
} catch (const std::exception& e) {
  std::println("exception bubbled up to main:\n{}", e.what());
  return 1;
}
