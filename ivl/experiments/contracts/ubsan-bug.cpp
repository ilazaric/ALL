#include <contracts>
#include <meta>
#include <string>
#include <utility>

template<typename E>
constexpr std::string enum_to_string(E e) {
  template for (constexpr auto enumerator : define_static_array(enumerators_of(^^E))) {
    if (e == extract<E>(enumerator)) return std::string(identifier_of(enumerator));
  }
  return "<not-enumerator[" + std::to_string(std::to_underlying(e)) + "]>";
}

void handle_contract_violation(const std::contracts::contract_violation& v) { (void)enum_to_string(v.kind()); }

// IVL add_compiler_flags("-fsanitize=undefined")
// IVL disable_ivl_main_handler()

int main() {}
