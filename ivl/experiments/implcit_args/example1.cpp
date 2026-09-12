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

// "implicit command line arguments" library -- suffix
int main(int argc, const char* const* argv) try {
  // must {instantiate before,run after} implicit_parse
  auto help = [] { return implicit_flag("help"); };
  std::span<const char* const> args(argv + !!argc, argv + argc);
  implicit_parse(args);
  if (!help()) return main2(args);
  std::println("Options:");
  template for (constexpr size_t index : std::views::iota(0ULL, implicit_size())) {
    static constexpr auto node = implicit_fetch(index);
    auto msg = define_static_string(std::format("  --{}: {}", node.name(), display_string_of(node.type)));
    std::println("{}", msg);
  }
  return 1;
} catch (const std::exception& e) {
  std::println("exception bubbled up to main:\n{}", e.what());
  return 2;
}
// ~ "implicit command line arguments" library -- suffix
