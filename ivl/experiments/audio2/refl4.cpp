#include <meta>

consteval {
consteval {
consteval {
consteval {
consteval {
  auto r = std::meta::reflect_constant_string("hello");
  __builtin_constexpr_diag(32, "", display_string_of(r));
  __builtin_constexpr_diag(32, "", display_string_of(parent_of(r)));
}
}
}
}
}
