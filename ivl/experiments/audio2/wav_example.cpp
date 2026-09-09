#include "wav"

int ivl_main(const std::filesystem::path& file) {
  auto p = ivl::wav::load(file);
  contract_assert(p.format_type == 1);
  save(p, "copy.wav");

  auto v = channel_split(p);
  auto q = ivl::wav::channel_merge(v);
  contract_assert(q == p);

  for (size_t i = 0; i < v.size(); ++i)
    save(v[i], std::format("channel_{}.wav", i));
  return 0;
}
