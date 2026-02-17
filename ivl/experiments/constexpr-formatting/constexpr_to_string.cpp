#include <format>
#include <map>
#include <meta>
#include <print>
#include <string>
#include <string_view>
#include <vector>

// constexpr std::string_view str = std::define_static_string(std::to_string(123));

int main() {
  // std::println("{}", "hello world");
  std::println("~~~\n{}", std::vector{1, 2, 3});
  // std::println("~~~\n{}", std::map<int, int>{{1,1}});

  // long l = 5L;
  // auto cstore = std::make_format_args<std::format_context>(l);
}
