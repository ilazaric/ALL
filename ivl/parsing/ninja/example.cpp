#include <ivl/parsing/ninja>
#include <ivl/reflection/json>

int ivl_main(const std::filesystem::path& p) {
  ivl::fmt::println("{:2}", ivl::to_json(ivl::parsing::ninja::parse(p)));
  return 0;
}
