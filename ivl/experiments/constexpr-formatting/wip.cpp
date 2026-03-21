#include <format>

constexpr bool test() {
  std::string_view spec("0{}d");
  std::format_parse_context pc(spec);
  pc._M_indexing = std::format_parse_context::_Auto;
  pc._M_next_arg_id = 1;
  std::__format::__formatter_int<char> f;
  f._M_do_parse(pc, decltype(f)::_AsChar);
  // std::formatter<char> f;
  // f.parse(pc);

  return true;
}

static_assert(test());

int main() {
  test();
}

// consteval {
//   std::string_view spec("0{}d");
//   std::format_parse_context pc(spec);
//   // (void)pc.next_arg_id();
//   // pc.check_arg_id(0);
//   pc._M_indexing = std::format_parse_context::_Auto;
//   std::formatter<char> f;
//   f.parse(pc);
// }
