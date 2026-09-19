// via friend injection

#include <meta>
#include <print>
#include <string>
#include <string_view>
#include <vector>

template<typename T>
struct A {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
  friend consteval auto fn(A);
#pragma GCC diagnostic pop
};

template<typename T, auto V>
struct B {
  friend consteval auto fn(A<T>) { return V; }
};

void bla() {
  consteval {
    {
      long x = 1;
      static constexpr auto r = ^^x;
      consteval {
        return; // dont work
        size_of(substitute((^^B), {(^^char), std::meta::reflect_constant(r)}));
      }
    }
    // [:fn(A<char>{}):] = 2;
    // __builtin_constexpr_diag(32, "", std::format("{}", [:fn(A<char>{}):]));
  }
}

int main() {
  bla();
}
