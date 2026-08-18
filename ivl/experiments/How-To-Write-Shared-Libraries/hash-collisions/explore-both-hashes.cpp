#include <ivl/logger>
#include "dl-hash"
#include <algorithm>
#include <string>
#include <vector>

// 0, 32, 64
// p = 33
// sum p^i * ai
// 0, p-1, 2p-2

constexpr uint32_t p = 33;

int ivl_main(int n) {
  {
    uint32_t curr = 1;
    uint32_t cnt = 0;
    do {
      curr *= p;
      ++cnt;
    } while (curr != 1);
    LOG(cnt);
  }

  // {
  //   std::vector<std::pair<uint32_t, std::string>> v{{0, ""}};
  //   for (int i = 0; i < n; ++i) {
  //     auto prev = v.size();
  //     for (int j = 0; j < prev; ++j) {
  //       v[j].first *= p;
  //       v[j].second += "0";
  //       v.push_back(v[j]);
  //       v.back().first += 32;
  //       v.back().second.back() = 'P';
  //       v.push_back(v[j]);
  //       v.back().first += 64;
  //       v.back().second.back() = 'p';
  //     }
  //   }
  //   for (auto&& [h, s] : v)
  //     if (h == 0) LOG(s);
  // }

  std::vector<std::string> geq{
    "ppp0Pp0pP0PPPp000P", "pp0PPPPPPP000p0pPp", "pp00p0PpP0P0PPpP0p", "pPp0P00pPpP00PPPpP", "pPPPp00P0PpPPPP0Pp",
    "pP00PPpPPPp0pp000p", "p0pPP0p00Pp0pPPPp0", "p00Ppp0p0Pp0P0ppP0", "Pp0pP00pp0Pp0p00Pp", "Pp00p0ppPP0PP0ppP0",
    "PPppP0ppPp0p00PP00", "PPp0PpP00PPPpPpp00", "PPPpPP0p0p00pP0pPP", "PPPp00pPp0PPPp0Pp0", "PPPPp0PP0pPpP0PPp0",
    "PPPP000PpPP00pppPp", "P0p0ppPPP0p00pPp0P", "P0p000p0pP0PppP0pp", "P00PPpppp00ppP00p0", "P000Pp0ppP0PPPpPPp",
    "0ppPPpP0p000P0PppP", "0pp00PpppP000PpPp0", "0pPp0pp0PP0P0p0PpP", "0pPPPPpPP0PPPPp0PP", "0pPP0pPPp00pPpPp00",
    "0p00Pp00P0PpppPpp0", "0P0pP0P0PPPpPPppPP", "0P00P00pppppPPp00p", "00P000pPpppPpPpPP0",
  };

  auto hex = [](uint32_t i) {
    std::string ret;
    for (int r = 0; r < 8; ++r) {
      auto c = i % 16;
      i /= 16;
      if (c <= 9) ret += (char)('0' + c);
      else ret += (char)('A' + c - 10);
    }
    std::ranges::reverse(ret);
    return ret;
  };

  {
    std::vector<std::pair<uint32_t, std::string>> v;
    for (auto s : geq) v.emplace_back(elf_hash(s.c_str()), s);
    std::ranges::sort(v);
    for (auto&& [h, s] : v) LOG(s, hex(h));
  }

  return 0;
}
