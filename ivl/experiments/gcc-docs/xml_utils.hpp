#pragma once

#include <pugixml/pugixml.hpp>
#include <ranges>
#include <set>
#include <string>
#include <vector>

namespace xml {
struct xml_string_writer : pugi::xml_writer {
  std::string result;
  virtual void write(const void* data, size_t size) { result.append(static_cast<const char*>(data), size); }
};

std::string to_string(pugi::xml_node node) {
  xml_string_writer writer;
  node.print(writer);
  return writer.result;
}

void recurse(pugi::xml_node node, auto&& pred) {
  for (auto it = node.begin(); it != node.end();) {
    auto child = *it;
    ++it;
    if (pred(child)) recurse(child, pred);
  }
}

void recurse_name(pugi::xml_node node, std::string_view name, auto&& pred) {
  recurse(node, [&](pugi::xml_node node) { return node.name() != name || pred(node); });
}

void assert_is_text(pugi::xml_node node) {
  contract_assert(node.name() == std::string_view(""));
  contract_assert(std::ranges::distance(node) == 0);
  contract_assert(std::ranges::distance(node.attributes()) == 0);
};

void assert_wraps_text(pugi::xml_node node) {
  contract_assert(std::ranges::distance(node) == 1);
  contract_assert(std::ranges::distance(node.attributes()) == 0);
  assert_is_text(node.first_child());
};

std::vector<std::pair<std::string, std::string>> attrs(pugi::xml_node node) {
  return node.attributes() | std::views::transform([](pugi::xml_attribute a) {
           return std::pair(std::string(a.name()), std::string(a.value()));
         }) |
         std::ranges::to<std::vector>();
}

std::size_t name_count(pugi::xml_node node) {
  std::set<std::string_view> names;
  recurse(node, [&](pugi::xml_node node) {
    names.insert(std::string_view(node.name()));
    return true;
  });
  return names.size();
}

std::string extract_first_check_name(pugi::xml_node node, std::string_view name) {
  auto child = node.first_child();
  contract_assert(child.name() == name);
  auto ret = xml::to_string(child);
  node.remove_child(child);
  return ret;
}

void purge_name(pugi::xml_node node, std::string_view name) {
  recurse_name(node, name, [](pugi::xml_node node) {
    node.parent().remove_child(node);
    return false;
  });
}
} // namespace xml
