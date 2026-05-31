#pragma once

#include <pugixml/pugixml.hpp>
#include <ranges>
#include <string>
#include <vector>

namespace xml {
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

struct xml_string_writer : pugi::xml_writer {
  std::string result;
  virtual void write(const void* data, size_t size) { result.append(static_cast<const char*>(data), size); }
};

std::string to_string(pugi::xml_node node) {
  xml_string_writer writer;
  node.print(writer);
  return writer.result;
}

std::vector<std::pair<std::string, std::string>> attrs(pugi::xml_node node) {
  return node.attributes() | std::views::transform([](pugi::xml_attribute a) {
           return std::pair(std::string(a.name()), std::string(a.value()));
         }) |
         std::ranges::to<std::vector>();
}
} // namespace xml
