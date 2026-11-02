#include <ivl/utility/enum>
#include <contracts>

namespace ivl {
struct contract_assertion_exception {};
} // namespace ivl

void handle_contract_violation(const std::contracts::contract_violation& v) {
  (void)ivl::enum_to_string(v.kind());
  throw ivl::contract_assertion_exception{};
}

// IVL add_compiler_flags("-fsanitize=undefined")
// IVL disable_ivl_main_handler()

int main() {
}
