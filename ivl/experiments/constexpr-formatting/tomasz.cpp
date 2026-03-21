#include <iostream>

#include <meta>

#define private public
#include <format>

static_assert(__cpp_lib_format >= 202305L);

namespace std {

consteval {
  if consteval {
    return;
  }
  char buffer[1000];
  auto ptr = std::format_to(buffer, "{:0}", 10);
  // throw std::meta::exception(std::format("{}", ptr-buffer), {});
  throw std::meta::exception(std::string_view(buffer, ptr), {});
}

constexpr void test() {
  int vec = 42;
  __format::_Str_sink<char> __buf;
  int x = 10;
  string_view __fmt = "{:{}}";
  auto argss = make_format_args(vec, x);
  format_args __args = argss;

  auto __ctx = format_context(__args, __buf.out());
  __format::_Formatting_scanner<__format::_Sink_iter<char>, char> __scanner(__ctx, __fmt);
  __scanner._M_scan();
}

consteval { test(); }

} // namespace std

int main() { std::test(); }
