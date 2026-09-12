#include <meta>

template<typename T>
struct S {
  template<typename = void>
  static consteval T foo() { return 42; }
  
  static consteval T bar() {
    return extract<T(*)()>(substitute(^^foo, {}))();
  }
};

static_assert(S<int>::bar() == 42);
