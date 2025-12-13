#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <ivl/fs/fileview>

std::vector<std::string> keywords{
  "constexpr", "print",
  // "condition",
  // "noexcept",
  // "contract",
  // "throw",
  // "exception",
};

int main() {
  std::vector<std::pair<std::string, double>> evals;
  for (auto f : std::filesystem::directory_iterator("index-elements")) {
    // std::cout << f << std::endl;
    // std::ifstream fin(f.path());
    // std::string cont(std::istreambuf_iterator<char>(fin), {});

    if (f.path().filename().string()[0] != 'P' && f.path().filename().string()[0] != 'D') continue;

    ivl::fs::FileView data(f.path().string());
    std::string_view  sv((const char*)data.mapped_region, data.length);
    // std::cout << " " << data.length << " " << f.path() << std::endl;

    if (not std::ranges::all_of(keywords, [&](std::string_view kw) { return sv.find(kw) != std::string_view::npos; }))
      continue;

    evals.emplace_back(f.path().filename().string(), 0);
  }

  std::ranges::sort(evals, {}, &std::pair<std::string, double>::second);
  for (auto [s, d] : evals)
    std::cout << s << " " << d << std::endl;
}
