#include <algorithm>
#include <cassert>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <ivl/logger>
#define IVL_DBG_MODE
#include <ivl/debug>

std::vector<std::string> stuff {"bnf", "bnfbase", "bnflist", "ncbnf", "ncrebnf", "ncsimplebnf"};

void validate(std::string_view sv) {
  for (auto& bla : stuff) {
    auto truc = "\\begin{" + bla + "}";
    auto it   = sv.find(truc);
    if (it == sv.npos)
      continue;
    if (sv == truc)
      continue;
    LOG(sv);
  }
}

void stage2(std::span<std::string> ssv) {
  for (auto& sv : ssv)
    std::cout << sv << " ";
  std::cout << std::endl;
}

int main() {
  for (auto d : std::filesystem::directory_iterator {"/home/ilazaric/repos/draft/source"}) {
    auto& f = d.path();
    if (f.extension() != ".tex")
      continue;
    if (f.filename() == "macros.tex")
      continue;
    if (f.filename() == "intro.tex")
      continue;
    LOG(f);

    std::ifstream            fin {f};
    std::vector<std::string> words;
    for (std::string s; std::getline(fin, s);) {
      std::string_view tail {s};
      if (auto loc = tail.find('%'); loc != tail.npos)
        tail = tail.substr(0, loc);

      while (!tail.empty()) {
        auto it  = std::ranges::find_if(tail, std::iswspace);
        auto loc = it == tail.end() ? tail.npos : it - tail.begin();
        if (loc)
          words.emplace_back(tail.substr(0, loc));
        if (loc == tail.npos)
          break;
        tail = tail.substr(loc + 1);
      }
    }

    std::size_t last_begin = -1;
    for (auto idx : std::views::iota(0ull, words.size())) {
      auto lo = words[idx].find('{');
      auto hi = words[idx].find('}');
      if (lo == std::string_view::npos)
        continue;
      if (hi == std::string_view::npos)
        continue;
      if (lo > hi)
        continue;
      // assert(hi != std::string_view::npos);
      // assert(lo < hi);
      auto kind = words[idx].substr(lo + 1, hi - lo - 1);
      if (std::ranges::find(stuff, kind) == stuff.end())
        continue;
      // LOG(f, kind, idx, words[idx]);

      if (words[idx].starts_with("\\begin{")) {
        IVL_DBG_ASSERT(last_begin == -1, f);
        last_begin = idx;
        continue;
      }

      if (words[idx].starts_with("\\end{")) {
        assert(last_begin != -1);
        // LOG(f);
        stage2(std::span(words).subspan(last_begin, idx - last_begin + 1));
        last_begin = -1;
        continue;
      }
    }

    IVL_DBG_ASSERT(last_begin == -1, f, words.size(), last_begin, words[last_begin]);
  }
}
