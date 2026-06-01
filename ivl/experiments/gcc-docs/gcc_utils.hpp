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
      std::string_view name = child.name();
      if (name == "urefurl") {
        xml::assert_wraps_text(child);
        node.append_attribute("href").set_value(child.text().get());
        node.prepend_move(child.first_child());
      } else if (name == "urefreplacement") {
        node.prepend_move(child.first_child());
      } else if (name == "urefdesc") {
        // TODO: weird
        node.prepend_move(child.first_child());
        // node.append_attribute("title").set_value(child.text().get());
      } else {
        contract_assert(false);
      }
    }
    node.set_name("a");
    while (std::ranges::distance(node) > 1) node.remove_child(node.last_child());
    return false;
  });
}

void replace_text_commands(pugi::xml_node node) {
  auto replace = [](std::string& s, std::string_view from, std::string_view to) {
    while (true) {
      auto loc = s.find(from);
      if (loc == std::string::npos) break;
      s = s.substr(0, loc) + to + s.substr(loc + from.size());
    }
  };
  auto replace_cmds = [&](std::string& s) {
    replace(s, "&arobase;", "@");
    replace(s, "&eosperiod;", ".");
    replace(s, "&noeos;", "");
    // replace(s, "&textrsquo;", "'");
    // replace(s, "&textldquo;", "\"");
    // replace(s, "&textrdquo;", "\"");
    replace(s, "&textrsquo;", R"(’)");
    replace(s, "&textldquo;", R"(“)");
    replace(s, "&textrdquo;", R"(”)");
    replace(s, "&textmdash;", R"(—)");
    replace(s, "&dots;", R"(…)");
    replace(s, "&copyright;", R"(©)");
    replace(s, "&tex;", R"(TeX)");
    replace(s, "&textndash;", R"(–)");
  };
  xml::recurse_name(node, "", [&](pugi::xml_node node) {
    std::string value = node.value();
    replace_cmds(value);
    node.set_value(value);
    return false;
  });
}

void purge_ref(pugi::xml_node node) {
  xml::recurse(node, [&](pugi::xml_node node) {
    std::string_view name = node.name();
    if (name == "xref" || name == "pxref") {
      if (!node.attribute("manual")) return true;
      contract_assert(node.child("xrefinfofile"));
      return true;
    }
    if (name != "xrefinfofile") return true;
    xml::assert_wraps_text(node);
    auto parent = node.parent();
    std::string_view pname = parent.name();
    contract_assert(pname == "xref" || pname == "pxref");
    auto at = parent.attribute("manual");
    contract_assert(at);
    contract_assert(std::string_view(at.value()) == node.text().get());
    parent.remove_child(node);
    return false;
  });
}

void purge_node(pugi::xml_node node) {
  xml::recurse_name(node, "node", [&](pugi::xml_node node) {
    auto next = node.next_sibling();
    contract_assert(next);
    contract_assert(next.name() != std::string_view("node"));
    auto nodename = node.first_child();
    contract_assert(nodename.name() == std::string_view("nodename"));
    std::string_view name1 = node.attribute("name").value();
    std::string_view name2 = nodename.text().get();
    // "G_002b_002b-and-GCC" != "G++ and GCC"
    // contract_assert(name1 == name2);
    bool check = next.prepend_attribute("ivl_nodename").set_value(name1.data());
    contract_assert(check);
    node.parent().remove_child(node);
    return false;
  });
}

void purge_sectiontitle(pugi::xml_node node) {
  xml::recurse_name(node, "sectiontitle", [](pugi::xml_node node) {
    auto parent = node.parent();
    contract_assert(node == parent.first_child());
    bool check = parent.prepend_attribute("ivl_sectiontitle").set_value(node.text().get());
    contract_assert(check);
    parent.remove_child(node);
    return false;
  });
}

void purge_menuleadingtext(pugi::xml_node node) {
  xml::recurse_name(node, "menuleadingtext", [](pugi::xml_node node) {
    contract_assert(xml::to_string(node) == "<menuleadingtext>* </menuleadingtext>\n");
    node.parent().remove_child(node);
    return false;
  });
}

void purge_columnfractions(pugi::xml_node node) {
  xml::recurse(node, [&](pugi::xml_node node) {
    std::string_view name = node.name();
    contract_assert(name != "columnfraction");
    if (name != "columnfractions") return true;
    auto at_line = node.attribute("line");
    contract_assert(at_line);
    contract_assert(std::ranges::distance(node.attributes()) == 1);
    std::string acc;
    for (auto child : node) {
      contract_assert(child.name() == std::string_view("columnfraction"));
      contract_assert(std::ranges::distance(child) == 0);
      auto at_value = child.attribute("value");
      contract_assert(at_value);
      contract_assert(std::ranges::distance(child.attributes()) == 1);
      acc += at_value.value();
      acc += " ";
    }
    if (!acc.empty()) acc.pop_back();
    contract_assert(at_line.value() == acc);
    node.remove_children();
    return false;
  });
  xml::recurse_name(node, "columnfractions", [&](pugi::xml_node node) {
    auto parent = node.parent();
    contract_assert(parent.name() == std::string_view("multitable"));
    auto at = parent.append_attribute("ivl_cf_line");
    at.set_value(node.attribute("line").value());
    parent.remove_child(node);
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

void transform_divlike(pugi::xml_node node) {
  xml::recurse_name(node, "center", [](pugi::xml_node node) {
    xml::assert_wraps_text(node);
    node.set_name("div");
    node.append_attribute("class").set_value("center");
    return false;
  });
  xml::recurse_name(node, "display", [](pugi::xml_node node) {
    contract_assert(std::ranges::distance(node.attributes()) == 0);
    node.set_name("div");
    node.append_attribute("class").set_value("display");
    return true;
  });
  xml::recurse_name(node, "smallexample", [](pugi::xml_node node) {
    contract_assert(std::ranges::distance(node.attributes()) == 0);
    node.set_name("div");
    node.append_attribute("class").set_value("example smallexample");
    return true;
  });
}

void transform_acronym(pugi::xml_node node) {
  xml::recurse_name(node, "acronym", [](pugi::xml_node node) {
    contract_assert(std::ranges::distance(node) == 1);
    contract_assert(std::ranges::distance(node.attributes()) == 0);
    xml::assert_wraps_text(node.first_child());
    contract_assert(node.first_child().name() == std::string_view("acronymword"));
    node.append_move(node.first_child().first_child());
    node.remove_child(node.first_child());
    node.set_name("abbr");
    node.append_attribute("class").set_value("acronym");
    return true;
  });
  xml::recurse_name(node, "acronymword", [](pugi::xml_node node) {
    contract_assert(false);
    return true;
  });
}

void transform_heading(pugi::xml_node node) {
  xml::recurse_name(node, "heading", [](pugi::xml_node node) {
    node.set_name("h3");
    return true;
  });
}

void transform_listlike(pugi::xml_node node) {
  xml::recurse(node, [](pugi::xml_node node) {
    std::string_view name = node.name();
    if (name == "enumerate") {
      node.set_name("ol");
      // LOG(std::format("{}", xml::attrs(node)));
      std::string first = node.attribute("first").value();
      node.remove_attribute("first");
      contract_assert(xml::attrs(node).size() == 0);
      contract_assert(std::ranges::distance(node));
      contract_assert(node.first_child().name() == std::string_view("enumeratefirst"));
      xml::assert_wraps_text(node.first_child());
      {
        std::string ef = node.first_child().text().get();
        node.remove_child(node.first_child());
        if (first.size() && ef.size()) contract_assert(first == ef);
        contract_assert(first.size() || ef.size());
        if (first.empty()) first = ef;
      }
      contract_assert(first.size() == 1);
      if ('a' <= first[0] && first[0] <= 'z') {
        node.append_attribute("type").set_value("a");
        node.append_attribute("start").set_value(std::to_string(first[0] - 'a' + 1));
      } else if ('A' <= first[0] && first[0] <= 'Z') {
        node.append_attribute("type").set_value("A");
        node.append_attribute("start").set_value(std::to_string(first[0] - 'A' + 1));
      } else if ('0' <= first[0] && first[0] <= '9') {
        node.append_attribute("type").set_value("1");
        node.append_attribute("start").set_value(std::to_string(first[0] - '0'));
      } else contract_assert(false);
      return true;
    }
    contract_assert(name != "enumeratefirst");
    if (name == "itemize") {
      node.set_name("ul");
      contract_assert(xml::attrs(node) == std::vector<std::pair<std::string, std::string>>{{"commandarg", "bullet"}});
      node.remove_attributes();
      node.append_attribute("class").set_value("itemize mark-bullet");
      return true;
    }
    if (name == "listitem") {
      node.set_name("li");
      // LOG(xml::to_string(node.first_child()));
      if (std::ranges::distance(node) && node.first_child().name() == std::string_view("prepend")) {
        contract_assert(xml::to_string(node.first_child()) == "<prepend>&amp;bullet;</prepend>\n");
        node.remove_child(node.first_child());
      }
      return true;
    }
    return true;
  });
}
} // namespace gcc
