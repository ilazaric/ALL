#pragma once

#include <ivl/utility/scope_exit>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <string_view>

namespace html {
std::ofstream* current_page = nullptr;
std::size_t current_depth = 0;

void emit_raw(std::string_view sv) { *current_page << std::string(current_depth * 2, ' ') << sv << "\n"; }

template<typename M = std::map<std::string_view, std::string_view>>
auto create_node_raii(std::string_view name, const M& attrs = {}) {
  contract_assert(current_page != nullptr);
  *current_page << std::string(current_depth * 2, ' ') << "<" << name;
  for (auto&& [name, value] : attrs) *current_page << std::format(" {}={:?}", name, value);
  *current_page << ">\n";
  ++current_depth;
  return ivl::util::scope_exit{[name = std::string(name)] {
    --current_depth;
    *current_page << std::string(current_depth * 2, ' ') << "</" << name << ">\n";
  }};
}

template<typename M = std::map<std::string_view, std::string_view>>
void create_node(std::string_view name, const M& attrs, auto&& cb) {
  auto _ = create_node_raii(name, attrs);
  cb();
}

void create_node(std::string_view name, auto&& cb) { create_node(name, {}, cb); }

void create_page(const std::filesystem::path& file, auto&& cb) {
  contract_assert(!exists(file));
  contract_assert(current_page == nullptr);
  std::ofstream fout(file);
  current_page = &fout;
  ivl::util::scope_exit _{[] { current_page = nullptr; }};
  fout << "<!DOCTYPE html>\n";
  create_node("html", cb);
}
} // namespace html
