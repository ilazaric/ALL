#include <ivl/command_line_argument_parsing/implicit_exposed>
#include "pseudo_boolean"
#include <ranges>

size_t count_true(std::span<const bool> data) { return std::ranges::count(data, true); }

int ivl_main(ivl::cmdline_parsing::implicit) {
  ivl::fmt::println(
    "{}",
    ivl::pseudo_boolean_optimize(implicit_value("n", 10'000UZ), implicit_value("rep", 10'000UZ), count_true).second
  );
  return 0;
}
