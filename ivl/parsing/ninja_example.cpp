#include <ivl/parsing/ninja>
#include <ivl/reflection/json>

int ivl_main(const std::filesystem::path& p) {
  std::println("{}", ivl::to_json(ivl::parsing::ninja::parse(p)).dump(2));
  // ivl::parsing::ninja::rule_variable::text foo = "bar";
  // std::println("{}", ivl::to_json(foo).dump(2));
  return 0;
}
