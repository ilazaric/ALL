// via friend injection

#include <meta>
#include <print>
#include <string>
#include <string_view>
#include <vector>

template<typename T>
struct A {
  friend consteval auto fn(A);
};

template<typename T, auto V>
struct B {
  friend consteval auto fn(A<T>) { return V; }
};

std::string_view bytes_of(const auto& arg) { //
  return std::string_view((const char*)&arg, (const char*)(&arg + 1));
}

void access_out_of_scope_static() {
  if (0) {
    static std::vector<std::string> x = {"hello", " ", "world", "\n"};
    consteval { size_of(substitute((^^B), {(^^int), std::meta::reflect_constant(^^x)})); }
  }
  std::println("{} -- {}", __func__, (const void*)&[:fn(A<int>{}):]);
  std::println("{} -- {:?}", __func__, bytes_of([:fn(A<int>{}):]));
}

void access_out_of_scope_automatic() {
  if (0) {
    long x = 1;
    static constexpr auto r = ^^x;
    consteval { size_of(substitute((^^B), {(^^char), std::meta::reflect_constant(r)})); }
  }
  std::println("{} -- {} {}", __func__, (const void*)&[:fn(A<char>{}):], [:fn(A<char>{}):]);
  [:fn(A<char>{}):] = {};
  std::println("{} -- {} {}", __func__, (const void*)&[:fn(A<char>{}):], [:fn(A<char>{}):]);
}

int main() {
  access_out_of_scope_static();
  access_out_of_scope_automatic();
}
