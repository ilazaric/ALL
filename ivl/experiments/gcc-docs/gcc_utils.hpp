#pragma once

#include "xml_utils"

namespace gcc {
void purge_duds(pugi::xml_node node) {
  const std::set<std::string_view> duds{
    "clear",
    "set",
    "bye",
    "documentlanguage",
    "sp",
    "title",
    "subtitle",
    "settitle",
    "paragraphindent",
    "author",
    "page",
    "vskip",
    "insertcopying",
    "filename",
    "copying",
    "preamblebeforebeginning",
    "setfilename",
    "dircategory",
    "setchapternewpage",
    "summarycontents",
    "contents",
    // define indexes
    "defcodeindex",
    "defindex",
  };
  xml::recurse(node, [&](pugi::xml_node node) {
    if (duds.contains(std::string_view(node.name()))) {
      node.parent().remove_child(node);
      return false;
    } else {
      return true;
    }
  });
}

void merge_cindex_indexterm(pugi::xml_node node) {
  // cindex always contains just an indexterm, merge them into ivl_cindex_indexterm
  xml::recurse_name(node, "cindex", [](pugi::xml_node node) {
    // LOG(xml::to_string(node));
    contract_assert(node.attribute("index").value() == std::string_view("cp"));
    // contract_assert(node.attribute("spaces").value() == std::string_view(" "));
    contract_assert(std::ranges::distance(node.attributes()) == 2 - 1);
    contract_assert(std::ranges::distance(node.children()) == 1);
    auto child = node.first_child();
    contract_assert(child.name() == std::string_view("indexterm"));
    contract_assert(child.attribute("index").value() == std::string_view("cp"));
    child.set_name("ivl_cindex_indexterm");
    node.parent().insert_move_before(child, node);
    node.parent().remove_child(node);
    return false;
  });
}

void merge_indexcommand_indexterm(pugi::xml_node node) {
  xml::recurse_name(node, "indexcommand", [](pugi::xml_node node) {
    std::string_view index = node.attribute("index").value();
    std::string_view command = node.attribute("command").value();
    contract_assert(index.size() == 2);
    contract_assert(command.starts_with(index));
    contract_assert(command.ends_with("index"));
    contract_assert(command.size() == index.size() + 5);
    contract_assert(std::ranges::distance(node.attributes()) == 2);
    contract_assert(std::ranges::distance(node.children()) == 1);
    auto child = node.first_child();
    contract_assert(child.name() == std::string_view("indexterm"));
    contract_assert(child.attribute("index").value() == index);
    // do it
    child.set_name("ivl_indexcommand_indexterm");
    node.parent().insert_move_before(child, node);
    node.parent().remove_child(node);
    return false;
  });
}
} // namespace gcc
