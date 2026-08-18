#include <ivl/logger>
#include "dl-hash"
#include <cstdint>
#include <format>
#include <map>
#include <vector>

int ivl_main(int n) {
  std::string allowed = "_";
  for (char c = 'a'; c <= 'z'; ++c) allowed += c;
  for (char c = 'A'; c <= 'Z'; ++c) allowed += c;
  for (char c = '0'; c <= '9'; ++c) allowed += c;
  LOG(allowed.size());

  std::map<uint32_t, std::vector<std::string>> h2s;
  std::string curr(n, '.');
  auto rec = [&](this auto&& self, int idx) {
    if (idx == n) {
      auto h = new_hash(curr.c_str());
      h2s[h].push_back(curr);
      return;
    }
    for (auto c : allowed) {
      curr[idx] = c;
      self(idx + 1);
    }
  };
  rec(0);

  std::vector<std::string> m;
  for (auto&& [k, v] : h2s)
    if (m.size() < v.size()) m = v;

  LOG(m.size(), std::format("{}", m));

  return 0;
}
