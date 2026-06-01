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

void transform_email(pugi::xml_node node) {
  xml::recurse(node, [](pugi::xml_node node) {
    contract_assert(node.name() != std::string_view("emailaddress"));
    if (node.name() != std::string_view("email")) return true;
    contract_assert(std::ranges::distance(node) == 1);
    contract_assert(std::ranges::distance(node.attributes()) == 0);
    contract_assert(node.first_child().name() == std::string_view("emailaddress"));
    xml::assert_wraps_text(node.first_child());
    node.parent().insert_move_before(node.first_child(), node);
    node = node.previous_sibling();
    node.parent().remove_child(node.next_sibling());
    node.set_name("a");
    node.append_attribute("class").set_value("email");
    std::string email = node.text().get();
    node.append_attribute("href").set_value("mailto:" + email);
    return false;
  });
}

void transform_url(pugi::xml_node node) {
  xml::recurse(node, [&](pugi::xml_node node) {
    if (node.name() != std::string_view("uref") && node.name() != std::string_view("url")) return true;
    // LOG(xml::to_string(node));
    contract_assert(std::ranges::distance(node.attributes()) == 0);
    for (auto child : node) {
      xml::assert_wraps_text(child);
      std::string_view name = child.name();
      if (name == "urefurl") {
        node.append_attribute("href").set_value(child.text().get());
        node.prepend_move(child.first_child());
      } else if (name == "urefreplacement") {
        node.prepend_move(child.first_child());
      } else if (name == "urefdesc") {
        node.append_attribute("title").set_value(child.text().get());
      } else {
        contract_assert(false);
      }
    }
    node.set_name("a");
    while (std::ranges::distance(node) > 1) node.remove_child(node.last_child());
    return false;
  });
}

void transform_accent(pugi::xml_node node) {
  xml::recurse_name(node, "accent", [](pugi::xml_node node) {
    if (xml::to_string(node) == R"html(<accent type="uml" bracketed="off">o</accent>
)html") {
      node.first_child().set_value(R"(ö)");
      return false;
    }
    if (xml::to_string(node) == R"html(<accent type="uml" bracketed="off">u</accent>
)html") {
      node.first_child().set_value(R"(ü)");
      return false;
    }
    if (xml::to_string(node) == R"html(<accent type="cedil">c</accent>
)html") {
      node.first_child().set_value(R"(ç)");
      return false;
    }
    if (xml::to_string(node) == R"html(<accent type="acute" bracketed="off">o</accent>
)html") {
      node.first_child().set_value(R"(ó)");
      return false;
    }
    if (xml::to_string(node) == R"html(<accent type="acute" bracketed="off">e</accent>
)html") {
      node.first_child().set_value(R"(é)");
      return false;
    }
    if (xml::to_string(node) == R"html(<accent type="acute" bracketed="off">a</accent>
)html") {
      node.first_child().set_value(R"(á)");
      return false;
    }
    if (xml::to_string(node) == R"html(<accent type="tilde" bracketed="off">n</accent>
)html") {
      node.first_child().set_value(R"(ñ)");
      return false;
    }
    LOG(xml::to_string(node));
    contract_assert(false);
    return true;
  });
}

void purge_space_attributes(pugi::xml_node node) {
  xml::recurse(node, [](pugi::xml_node node) {
    auto a = node.attribute("spaces");
    if (a) node.remove_attribute(a);
    a = node.attribute("endspaces");
    if (a) node.remove_attribute(a);
    return true;
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
