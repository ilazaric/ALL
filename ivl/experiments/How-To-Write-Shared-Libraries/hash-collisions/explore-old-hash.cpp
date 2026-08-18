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

  {
    std::array<std::string, 8> chars;
    for (int c : allowed) {
      int high = c / 16;
      int low = c % 16;
      LOG(c, high, low);
      for (int l = 0; l < 8; ++l) {
        if (high <= 7 - l && low <= l) chars[l] += (char)c;
      }
    }
    for (int i = 0; i < 8; ++i) {
      LOG(i, chars[i].size(), chars[i]);
    }
  }

  std::map<uint32_t, std::vector<std::string>> h2s;
  std::string curr(n + 1, '_');
  auto rec = [&](this auto&& self, int idx) {
    if (idx == n) {
      auto h = elf_hash(curr.c_str());
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

  auto hex = [](uint32_t i) {
    std::string ret;
    for (int r = 0; r < 8; ++r) {
      auto c = i % 16;
      i /= 16;
      if (c <= 9) ret += (char)('0' + c);
      else ret += (char)('A' + c - 10);
    }
    return ret;
  };

  LOG(hex(elf_hash("p")));
  LOG(hex(elf_hash("p0")));
  LOG(hex(elf_hash("p000000000000")));
  LOG(hex(elf_hash("ppppppppppppp")));
  LOG(hex(elf_hash("pPpppppPppppp")));

  return 0;
}
