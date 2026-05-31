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
  create_directories(file.parent_path());
  std::ofstream fout(file);
  current_page = &fout;
  ivl::util::scope_exit _{[] { current_page = nullptr; }};
  fout << "<!DOCTYPE html>\n";
  create_node("html", cb);
}

void create_cppref_head(std::string_view sv) {
  create_node("head", [&] {
    emit_raw(R"html(<meta charset="UTF-8">)html");
    create_node("title", [&] {
      if (sv.empty()) emit_raw("ivlreference");
      else emit_raw(std::format("{} - ivlreference", sv));
    });
    emit_raw(R"html(<link rel="stylesheet" href="/reference/style1.css">)html");
    emit_raw(R"html(<link rel="stylesheet" href="/reference/style2.css">)html");
  });
}

void create_cppref_header() {
  auto _ = create_node_raii("div", {{"id", "mw-head"}, {"class", "noprint"}});
  {
    auto _ = create_node_raii("div", {{"id", "cpp-head-first-base"}});
    auto _ = create_node_raii("div", {{"id", "cpp-head-first"}});
    {
      auto _ = create_node_raii("h5");
      auto _ = create_node_raii("a", {{"href", "/reference"}});
      emit_raw("ivlreference");
    }
    {
      auto _ = create_node_raii("div", {{"id", "cpp-head-search"}});
      auto _ = create_node_raii("div", {{"id", "p-search"}});
      emit_raw("<h5>Search</h5>");
      auto _ = create_node_raii("span");
      emit_raw("No search either");
    }
    {
      auto _ = create_node_raii("div", {{"id", "cpp-head-personal"}});
      auto _ = create_node_raii("div", {{"id", "p-personal"}, {"class", ""}});
      auto _ = create_node_raii("span", {{"id", "pt-createaccount"}});
      emit_raw("No accounts");
    }
  }
  auto _ = create_node_raii("div", {{"id", "cpp-head-second-base"}});
  auto _ = create_node_raii("div", {{"id", "cpp-head-second"}});
  {
    auto _ = create_node_raii("div", {{"id", "cpp-head-tools-left"}});
    emit_raw("TODO");
  }
  {
    auto _ = create_node_raii("div", {{"id", "cpp-head-tools-right"}});
    emit_raw("TODO");
  }
}

void create_cppref_page(const std::filesystem::path& file, std::string_view title, auto&& cb) {
  create_page(file, [&] {
    create_cppref_head(title);
    auto _ = create_node_raii(
      "body",
      {{"class",
        "mediawiki ltr sitedir-ltr mw-hide-empty-elt ns-0 ns-subject page-cpp_language_value_category rootpage-cpp "
        "skin-cppreference2 action-view cpp-navbar"}}
    );
    create_cppref_header();
    auto _ = create_node_raii("div", {{"id", "cpp-content-base"}});
    auto _ = create_node_raii("div", {{"id", "content"}, {"class", "mw-body"}});
    emit_raw(std::format(R"html(<h1 id="firstHeading" class="firstHeading">{}</h1>)html", title));
    auto _ = create_node_raii("div", {{"id", "bodyContent"}, {"class", "mw-body-content"}});
    auto _ = create_node_raii("div", {{"id", "mw-content-text"}});
    auto _ = create_node_raii("div", {{"class", "mw-content-ltr mw-parser-output"}, {"lang", "en"}, {"dir", "ltr"}});
    cb();
  });
}
} // namespace html
