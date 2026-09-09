#include "wav"

int ivl_main(const std::filesystem::path& file) {
  auto p = ivl::wav::load(file);
  // contract_assert(p.format_type == 1);
  ivl::wav::save(p, "copy.wav");
  return 0;
}
