#include <ivl/logger>
#include "dl-hash"
#include <cstdint>
#include <string>
#include <vector>

int ivl_main(int n) {
  std::string allowed = "pP0";
  LOG(allowed.size());

  std::vector<unsigned char> counts(1ULL << 32);
  std::string curr(n, '.');
  auto rec = [&](this auto&& self, int idx) {
    if (idx == n) {
      auto h = new_hash(curr.c_str());
      if (h == 1076879333u) LOG(curr);
      ++counts[h];
      return;
    }
    for (auto c : allowed) {
      curr[idx] = c;
      self(idx + 1);
    }
  };
  rec(0);

  uint32_t m = 0;
  for (size_t i = 0; i < counts.size(); ++i)
    if (counts[i] > counts[m]) m = i;
  LOG(m, (int)counts[m]);

  return 0;
}
