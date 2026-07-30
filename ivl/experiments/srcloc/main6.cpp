#include <source_location>
#include <meta>

constexpr auto* addr = __builtin_source_location();
constexpr auto& ref = *__builtin_source_location();

template<auto*>
void fn_ptr() {}

template<auto&>
void fn_ref() {}

int main() {
  fn_ptr<std::define_static_object(*__builtin_source_location())>();
  // fn_ref<*__builtin_source_location()>();
}
