#include "make"

int ivl_main(std::filesystem::path file) {
  auto state = ivl::parsing::make::parse(file);
  return 0;
}
