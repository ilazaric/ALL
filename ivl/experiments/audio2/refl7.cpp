#include <meta>

template<int>
struct S;

consteval std::meta::info get() {
  int i = 0;
  while (true) {
    auto r = std::meta::reflect_constant(i);
    auto t = substitute((^^S), {r});
    if (!is_complete_type(t)) return t;
    ++i;
  }
}

consteval {
  
}
