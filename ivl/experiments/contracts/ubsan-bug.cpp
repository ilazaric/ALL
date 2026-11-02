// #include <ivl/contracts/violation_handler>

// #pragma once

// #include <ivl/format/stacktrace>
#include <ivl/format>
#include <ivl/utility/enum>
#include <contracts>
// #include <stacktrace>

namespace ivl {
struct contract_assertion_exception {};
} // namespace ivl

void handle_contract_violation(const std::contracts::contract_violation& v) {
  using ivl::fmt::println;
  println(stderr, "contract violation occurred");
  println(stderr, "  kind: {}", ivl::enum_to_string(v.kind()));
  // println(stderr, "  detection mode: {}", ivl::enum_to_string(v.detection_mode()));
  // println(stderr, "  comment: {}", v.comment());
  // auto loc = v.location();
  // if (loc.line() == 0) println(stderr, "  location: <NULL>");
  // else println(stderr, "  location: {}:{} {}", loc.file_name(), loc.line(), loc.function_name());
  // println(stderr, "  stacktrace:\n{}\n", std::stacktrace::current());
  throw ivl::contract_assertion_exception{};
}

// IVL add_compiler_flags("-fsanitize=undefined")

// enum class E { A, B, C };

// IVL disable_ivl_main_handler()

int main() {
  // E e = E::B;
  // ivl::enum_to_string(e);
  // for (auto s : ivl::views::split_py(std::string("   hello    world  123   "))) ivl::fmt::println("[{}]", s);
}
