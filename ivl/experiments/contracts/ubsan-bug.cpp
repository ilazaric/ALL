#include <meta>
#include <utility>

template<typename E>
constexpr E foo(E e) {
  template for (constexpr auto er : define_static_array(enumerators_of(^^E))) {
    constexpr auto e2 = extract<E>(er);
    if (e == e2) return e2;
  }
  throw; // not reached
}

// IVL add_compiler_flags("-fsanitize=undefined")
// IVL disable_ivl_main_handler()

enum class E { A, B, C };

int main() {
  E e = E::B;
  (void)foo(e);
}

// compile with:
// g++ -std=c++26 -freflection ubsan-bug.cpp -fsyntax-only -fsanitize=undefined
