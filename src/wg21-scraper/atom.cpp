#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <utility>
#include <string_view>
#include <ranges>
#include <algorithm>
#include <cmath>

#include <ivl/fs/fileview.hpp>

std::vector<std::string> keywords{
  "violation",
  "post",
  "condition",
  "noexcept",
  "contract",
  "throw",
  "exception",
};

int main(){
  std::vector<std::pair<std::string, double>> evals;
  for (auto f : std::filesystem::directory_iterator("index-elements")){
    // std::cout << f << std::endl;
    // std::ifstream fin(f.path());
    // std::string cont(std::istreambuf_iterator<char>(fin), {});

    // if (f.path().filename().string()[0] != 'P' &&
    //     f.path().filename().string()[0] != 'D')
    //   continue;

    ivl::fs::FileView data(f.path().string());
    std::string_view sv((const char*)data.mapped_region, data.length);
    // std::cout << " " << data.length << " " << f.path() << std::endl;

    if (not std::ranges::all_of(keywords, [&](std::string_view kw){
      return sv.find(kw) != std::string_view::npos;
    })) continue;

    auto generate = [&](std::string_view kw){
      std::vector<std::size_t> locs;
      for (std::size_t loc = -1; (loc = sv.find(kw, loc+1)) != std::string_view::npos; locs.push_back(loc));
      return locs;
    };

    auto plocs = generate("post");
    auto nlocs = generate("noexcept");

    double delta = 1e20;
    for (auto p : plocs)
      for (auto n : nlocs)
        delta = std::min(delta, std::abs((double)p - n));
    evals.emplace_back(f.path().filename().string(), delta);
  }

  std::ranges::sort(evals, {}, &std::pair<std::string, double>::second);
  for (auto [s, d] : evals)
    std::cout << s << " " << d << std::endl;
}
