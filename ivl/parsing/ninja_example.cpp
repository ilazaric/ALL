#include <ivl/parsing/ninja>

int ivl_main(const std::filesystem::path& p) try {
  ivl::parsing::ninja::parse(p);
  return 0;
} catch (const ivl::base_exception& e) {
  e.dump();
  return 1;
}
