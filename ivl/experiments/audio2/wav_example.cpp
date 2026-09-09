#include "wav"

int ivl_main(const std::filesystem::path& file) {
  ivl::wav::load(file);
  return 0;
}
