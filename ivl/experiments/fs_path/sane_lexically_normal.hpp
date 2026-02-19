#include <filesystem>
#include <iostream>
#include <ranges>

namespace impl_sane {
std::filesystem::path lexically_normal(const std::filesystem::path& self) {
  std::filesystem::path ret;
  bool last_dot = false;
  if (self.empty()) return ret;

  std::vector<const std::filesystem::path*> pieces;
  auto my_append = [&](const path& p) { pieces.push_back(&p); };
  auto my_has_filename = [&] {
    if (pieces.empty()) return false;
    return true;
  };
  auto my_remove_filename = [&] {
    pieces.pop_back();
    last_dot = true;
  };
  auto my_filename = [&] { return *pieces.back(); };
  auto is_dotdot = [](const std::filesystem::path& p) { return p.native() == ".."; };
  auto is_dot = [](const std::filesystem::path& p) { return p.native() == "."; };

  if (is_dot(self)) return self;

  for (auto& p : std::views::drop(self, self.is_absolute())) {
    last_dot = false;
    if (is_dotdot(p)) {
      if (my_has_filename()) {
        if (!is_dotdot(my_filename())) my_remove_filename();
        else my_append(p);
      } else {
        if (!self.has_root_directory()) my_append(p);
      }
    } else if (is_dot(p)) last_dot = true;
    else my_append(p);
  }

  auto runit = [&](auto&& callback) {
    if (self.is_absolute()) callback(self.begin()->native());
    for (size_t i = 0; i < pieces.size(); ++i) {
      if (i != 0) callback("/"sv);
      callback(pieces[i]->native());
    }
    if (last_dot && !pieces.empty()) callback("/"sv);
  };

  size_t total = 0;
  runit([&](auto&& str) { total += std::ranges::size(str); });
  std::string rstr;
  rstr.reserve(total);
  runit([&](auto&& str) { rstr += str; });
  ret = std::move(rstr);

  if (std::ranges::distance(ret) >= 2) {
    auto back = std::prev(ret.end());
    // If the last filename is dot-dot, ...
    if (back->empty() && is_dotdot(*std::prev(back)))
      // ... remove any trailing directory-separator.
      ret = ret.parent_path();
  }
  // If the path is empty, add a dot.
  else if (ret.empty())
    ret = ".";

  return ret;
}
} // namespace impl_sane
