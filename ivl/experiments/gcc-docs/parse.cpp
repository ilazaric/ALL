#include <ivl/logger>
#include <ivl/utility>
#include "html_gen"
#include <pugixml/pugixml.hpp>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <print>
#include <ranges>
#include <set>
#include <source_location>

void describe(const pugi::xml_node& node, std::size_t current_depth, std::size_t max_depth) {
  std::println("{:<{}}{}", "", current_depth * 2, node.name());
  if (current_depth == max_depth) return;
  for (auto&& el : node) describe(el, current_depth + 1, max_depth);
}

struct xml_string_writer : pugi::xml_writer {
  std::string result;
  virtual void write(const void* data, size_t size) { result.append(static_cast<const char*>(data), size); }
};

std::string stringify(const pugi::xml_node& node) {
  xml_string_writer writer;
  node.print(writer);
  return writer.result;
}

void find_nodes_impl(pugi::xml_node node, std::string_view name, std::vector<pugi::xml_node>& out) {
  if (node.name() == name) {
    out.push_back(node);
    return;
  }
  for (auto&& child : node) find_nodes_impl(child, name, out);
}

std::vector<pugi::xml_node> find_nodes(pugi::xml_node node, std::string_view name) {
  std::vector<pugi::xml_node> ret;
  find_nodes_impl(node, name, ret);
  return ret;
}

void find_nodes_p_impl(pugi::xml_node node, auto&& pred, std::vector<pugi::xml_node>& out) {
  if (pred(node)) {
    out.push_back(node);
    return;
  }
  for (auto&& child : node) find_nodes_p_impl(child, pred, out);
}

std::vector<pugi::xml_node> find_nodes_p(pugi::xml_node node, auto&& pred) {
  std::vector<pugi::xml_node> ret;
  find_nodes_p_impl(node, pred, ret);
  return ret;
}

#define EXPECT(node, cond) expect(node, cond, #cond)

struct args {
  std::filesystem::path file;
  // std::size_t depth;
  std::filesystem::path output;
};

static_assert(sizeof(pugi::xml_node) == 8);

struct selfref {
  std::size_t count = 0;
  std::map<std::string, selfref> children;
};

void populate_selfref(selfref& top, pugi::xml_node node) {
  ++top.count;
  for (auto child : node) populate_selfref(top.children[std::string(child.name())], child);
}

void display_selfref(const selfref& top, std::string_view name, std::size_t depth) {
  std::println("{:<{}}{} ({})", "", depth * 2, name, top.count);
  for (auto&& [nc, cc] : top.children) display_selfref(cc, nc, depth + 1);
}

struct edges {
  std::map<std::string, std::map<std::string, std::size_t>> e;
};

void populate_edges(edges& e, pugi::xml_node node) {
  for (auto child : node) {
    ++e.e[std::string(node.name())][std::string(child.name())];
    populate_edges(e, child);
  }
}

void display_edges(const edges& e) {
  for (auto&& [l, rc] : e.e)
    for (auto&& [r, c] : rc) std::println("{} -> {} ({})", l, r, c);
}

std::vector<std::pair<std::string, std::string>> attrs(pugi::xml_node node) {
  return node.attributes() | std::views::transform([](pugi::xml_attribute a) {
           return std::pair(std::string(a.name()), std::string(a.value()));
         }) |
         std::ranges::to<std::vector>();
}

void path(pugi::xml_node node) {
  while (true) {
    std::println("{} {}", node.name(), node.attributes() | std::views::transform([](pugi::xml_attribute a) {
                                         return std::pair(a.name(), a.value());
                                       }));
    auto parent = node.parent();
    if (parent.empty()) break;
    auto loc = std::distance(parent.begin(), std::ranges::find(parent, node));
    contract_assert(loc != std::ranges::distance(parent));
    std::println("INDEX: {}", loc);
    node = parent;
  }
}

struct command_options {
  // very useful, paras and cindices
  std::vector<std::string> intro;
  // less useful prob
  std::string menu;
  // mountains of good stuff
  std::vector<std::string> sections;
};

struct top_level {
  // dunno if important
  std::vector<std::string> macros;
  std::vector<std::string> syncodeindexs;
  std::string direntry;
  std::string titlepage;

  // might be useful
  std::string top;

  // shouldnt be useful
  std::string appendix;

  // has some paras, but seems nondescript
  std::string supported_languages;

  // has a lot more paras, will need to think if info can be extracted
  std::string supported_standards;

  command_options opts;
};

int ivl_main(const args& args) {
  contract_assert(!args.output.empty());
  contract_assert(is_directory(args.output));
  auto output_dir = args.output / "reference" / "gcc";
  remove_all(output_dir);
  create_directories(output_dir);

  // for (auto file : {"style1.css", "style2.css", "fonts", "resources", "skins", "mwiki"})
  //   copy(
  //     std::filesystem::canonical("/proc/self/exe").parent_path() / file, args.output / "reference" / file,
  //     std::filesystem::copy_options::recursive
  //   );

  pugi::xml_document doc;
  if (!doc.load_file(args.file.native().c_str())) return 1;

  auto xml_recurse = [](this auto&& self, pugi::xml_node node, auto&& pred) -> void {
    for (auto it = node.begin(); it != node.end();) {
      auto child = *it;
      ++it;
      if (pred(child)) self(child, pred);
    }
  };

  auto name_count = [&](pugi::xml_node node) {
    std::set<std::string_view> names;
    xml_recurse(node, [&](pugi::xml_node node) {
      names.insert(std::string_view(node.name()));
      return true;
    });
    return names.size();
  };
  LOG(stringify(doc).size());
  LOG(name_count(doc));

  top_level t;

  auto texinfo = doc.first_child();
  contract_assert(texinfo.name() == std::string_view("texinfo"));

  if (1) {
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
    xml_recurse(doc, [&](pugi::xml_node node) {
      if (duds.contains(std::string_view(node.name()))) {
        node.parent().remove_child(node);
        return false;
      } else {
        return true;
      }
    });
  }

  xml_recurse(texinfo, [&](pugi::xml_node node) {
    if (node.name() == std::string_view("macro")) {
      t.macros.push_back(stringify(node));
      node.parent().remove_child(node);
    } else if (node.name() == std::string_view("syncodeindex")) {
      t.syncodeindexs.push_back(stringify(node));
      node.parent().remove_child(node);
    }
    return false;
  });

  auto take_first_check_name = [](pugi::xml_node node, std::string_view name) {
    auto child = node.first_child();
    contract_assert(child.name() == name);
    auto ret = stringify(child);
    node.remove_child(child);
    return ret;
  };

  t.direntry = take_first_check_name(texinfo, "direntry");
  t.titlepage = take_first_check_name(texinfo, "titlepage");

  xml_recurse(doc, [&](pugi::xml_node node) {
    if (node.name() != std::string_view("emailaddress")) return true;
    contract_assert(node.parent().name() == std::string_view("email"));
    contract_assert(std::ranges::distance(node) == 1);
    contract_assert(std::ranges::distance(node.parent()) == 1);
    contract_assert(std::ranges::distance(node.attributes()) == 0);
    contract_assert(std::ranges::distance(node.parent().attributes()) == 0);
    contract_assert(node.first_child().name() == std::string_view(""));
    return true;
  });

  xml_recurse(doc, [&](pugi::xml_node node) {
    if (node.name() == std::string_view("para")) node.set_name("p");
    return true;
  });

  // email.emailaddress -> emailaddress
  xml_recurse(doc, [&](pugi::xml_node node) {
    if (node.name() != std::string_view("email")) return true;
    auto child = node.first_child();
    auto parent = node.parent();
    parent.insert_move_before(child, node);
    parent.remove_child(node);
    return false;
  });

  xml_recurse(doc, [&](pugi::xml_node node) {
    if (node.name() != std::string_view("node")) return true;
    auto next = node.next_sibling();
    contract_assert(next);
    contract_assert(next.name() != std::string_view("node"));
    auto nodename = node.first_child();
    contract_assert(nodename.name() == std::string_view("nodename"));
    std::string_view name1 = node.attribute("name").value();
    std::string_view name2 = nodename.text().get();
    // "G_002b_002b-and-GCC" != "G++ and GCC"
    // contract_assert(name1 == name2);
    bool check = next.prepend_attribute("ivl_nodename").set_value(name2.data());
    contract_assert(check);
    node.parent().remove_child(node);
    return false;
  });

  xml_recurse(doc, [](pugi::xml_node node) {
    if (node.name() != std::string_view("sectiontitle")) return true;
    auto parent = node.parent();
    contract_assert(node == parent.first_child());
    bool check = parent.prepend_attribute("ivl_sectiontitle").set_value(node.text().get());
    contract_assert(check);
    parent.remove_child(node);
    return false;
  });

  xml_recurse(doc, [](pugi::xml_node node) {
    if (node.name() != std::string_view("menuleadingtext")) return true;
    contract_assert(stringify(node) == "<menuleadingtext>* </menuleadingtext>\n");
    node.parent().remove_child(node);
    return false;
  });

  t.top = take_first_check_name(texinfo, "top");

  {
    auto child = texinfo.last_child();
    contract_assert(child.name() == std::string_view("appendix"));
    t.appendix = stringify(child);
    texinfo.remove_child(child);
  }

  {
    auto child = texinfo.first_child();
    contract_assert(child.name() == std::string_view("chapter"));
    contract_assert(
      child.attribute("ivl_sectiontitle").value() == std::string_view("Programming Languages Supported by GCC")
    );
    t.supported_languages = stringify(child);
    texinfo.remove_child(child);
  }

  {
    auto child = texinfo.first_child();
    contract_assert(child.name() == std::string_view("chapter"));
    contract_assert(
      child.attribute("ivl_sectiontitle").value() == std::string_view("Language Standards Supported by GCC")
    );
    t.supported_standards = stringify(child);
    texinfo.remove_child(child);
  }

  xml_recurse(doc, [](pugi::xml_node node) {
    auto a = node.attribute("spaces");
    if (a) node.remove_attribute(a);
    return true;
  });

  // cindex always contains just an indexterm, merge them into ivl_cindex_indexterm
  xml_recurse(doc, [](pugi::xml_node node) {
    if (node.name() != std::string_view("cindex")) return true;
    // LOG(stringify(node));
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

  xml_recurse(doc, [](pugi::xml_node node) {
    if (node.name() != std::string_view("indexcommand")) return true;
    // LOG(stringify(node));
    // contract_assert(node.attribute("index").value() == std::string_view("op"));
    // contract_assert(node.attribute("command").value() == std::string_view("opindex"));
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

  xml_recurse(doc, [&](pugi::xml_node node) {
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

  xml_recurse(doc, [&](pugi::xml_node node) {
    std::string_view name = node.name();
    if (name != "columnfractions") return true;
    auto parent = node.parent();
    contract_assert(parent.name() == std::string_view("multitable"));
    auto at = parent.append_attribute("ivl_cf_line");
    at.set_value(node.attribute("line").value());
    parent.remove_child(node);
    return false;
  });

  auto unhex = [](char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    contract_assert(false);
    return -1;
  };

  auto deuglify = [&](std::string_view sv) {
    std::string ret;
    while (!sv.empty()) {
      if (sv[0] == '-') {
        ret.push_back(' ');
        sv.remove_prefix(1);
        continue;
      }
      if (sv[0] == '_') {
        contract_assert(sv.size() >= 5);
        contract_assert(sv[1] == '0');
        contract_assert(sv[2] == '0');
        ret.push_back((char)(unhex(sv[3]) * 16 + unhex(sv[4])));
        sv.remove_prefix(5);
        continue;
      }
      ret.push_back(sv[0]);
      sv.remove_prefix(1);
    }
    return ret;
  };

  auto normalize_text = [](std::string_view sv) {
    std::string ret;
    while (!sv.empty()) {
      if (sv[0] == '\n') {
        ret.push_back(' ');
        sv.remove_prefix(1);
        continue;
      }
      ret.push_back(sv[0]);
      sv.remove_prefix(1);
    }
    return ret;
  };

  auto assert_is_text = [](pugi::xml_node node) {
    contract_assert(node.name() == std::string_view(""));
    contract_assert(std::ranges::distance(node) == 0);
    contract_assert(std::ranges::distance(node.attributes()) == 0);
  };

  auto assert_wraps_text = [&](pugi::xml_node node) {
    contract_assert(std::ranges::distance(node) == 1);
    contract_assert(std::ranges::distance(node.attributes()) == 0);
    assert_is_text(node.first_child());
  };

  xml_recurse(doc, [&](pugi::xml_node node) {
    std::string_view name = node.name();
    if (name == "xref" || name == "pxref") {
      if (!node.attribute("manual")) return true;
      contract_assert(node.child("xrefinfofile"));
      return true;
    }
    if (name != "xrefinfofile") return true;
    assert_wraps_text(node);
    auto parent = node.parent();
    std::string_view pname = parent.name();
    contract_assert(pname == "xref" || pname == "pxref");
    auto at = parent.attribute("manual");
    contract_assert(at);
    contract_assert(std::string_view(at.value()) == node.text().get());
    parent.remove_child(node);
    return false;
  });

  xml_recurse(doc, [](pugi::xml_node node) {
    auto a = node.attribute("endspaces");
    if (a) node.remove_attribute(a);
    return true;
  });

  {
    auto chapter = texinfo.first_child();
    contract_assert(chapter.name() == std::string_view("chapter"));
    contract_assert(chapter.attribute("ivl_sectiontitle").value() == std::string_view("GCC Command Options"));
    auto child = chapter.first_child();
    while (child) {
      std::string_view name = child.name();
      if (name != "p" && name != "ivl_cindex_indexterm") break;
      t.opts.intro.push_back(stringify(child));
      child = child.next_sibling();
    }
    contract_assert(child.name() == std::string_view("menu"));
    t.opts.menu = stringify(child);
    child = child.next_sibling();
    while (child) {
      contract_assert(child.name() == std::string_view("section"));
      t.opts.sections.push_back(stringify(child));
      child = child.next_sibling();
    }
    texinfo.remove_child(chapter);
  }

  {
    auto node = texinfo.last_child();
    contract_assert(node.name() == std::string_view("unnumbered"));
    contract_assert(node.attribute("ivl_sectiontitle").value() == std::string_view("Contributors to GCC"));
    std::map<std::string, std::size_t> names;
    xml_recurse(node, [&](pugi::xml_node node) {
      ++names[std::string(node.name())];
      return true;
    });
    // for (auto&& [name, count] : names) LOG(name, count);
    // texinfo.remove_child(node);
  }

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
    replace(s, "&textrsquo;", "'");
    replace(s, "&textldquo;", "\"");
    replace(s, "&textrdquo;", "\"");
    // replace(s, "&textrsquo;", R"(’)");
    // replace(s, "&textldquo;", R"(“)");
    // replace(s, "&textrdquo;", R"(”)");
    replace(s, "&textmdash;", R"(—)");
    replace(s, "&dots;", R"(…)");
  };

  xml_recurse(doc, [&](pugi::xml_node node) {
    if (node.name() != std::string_view("")) return true;
    std::string value = node.value();
    replace_cmds(value);
    node.set_value(value);
    return false;
  });

  xml_recurse(doc, [&](pugi::xml_node node) {
    if (node.name() != std::string_view("emailaddress")) return true;
    node.set_name("a");
    node.append_attribute("class").set_value("email");
    std::string email = node.text().get();
    // LOG(email);
    // replace_cmds(email);
    // LOG(email);
    node.append_attribute("href").set_value("mailto:" + email);
    // node.first_child().set_value(email);
    return false;
  });

  html::create_cppref_page(output_dir / "contributors" / "index.html", "Contributors to GCC", [&] {
    auto node = texinfo.last_child();
    contract_assert(node.name() == std::string_view("unnumbered"));
    contract_assert(node.attribute("ivl_sectiontitle").value() == std::string_view("Contributors to GCC"));
    // html::create_cppref_head("Contributors to GCC");
    // auto _ = html::create_node_raii(
    //   "body",
    //   {{"class",
    //     "mediawiki ltr sitedir-ltr mw-hide-empty-elt ns-0 ns-subject page-cpp_language_value_category rootpage-cpp "
    //     "skin-cppreference2 action-view cpp-navbar"}}
    // );
    // html::create_cppref_header();
    // auto _ = html::create_node_raii("div", {{"id", "cpp-content-base"}});
    // auto _ = html::create_node_raii("div", {{"id", "content"}, {"class", "mw-body"}});
    // html::emit_raw(R"html(<h1 id="firstHeading" class="firstHeading">Contributors to GCC</h1>)html");
    // auto _ = html::create_node_raii("div", {{"id", "bodyContent"}, {"class", "mw-body-content"}});
    contract_assert(node.first_child().name() == std::string_view("ivl_cindex_indexterm"));
    node.remove_child(node.first_child());
    xml_recurse(node, [](pugi::xml_node node) {
      std::string_view name = node.name();
      if (name == "beforefirstitem" || name == "itemprepend") {
        node.parent().remove_child(node);
        return false;
      }
      if (name == "itemize") {
        node.set_name("ul");
        contract_assert(attrs(node) == std::vector<std::pair<std::string, std::string>>{{"commandarg", "bullet"}});
        node.remove_attributes();
        node.append_attribute("class").set_value("itemize mark-bullet");
        return true;
      }
      if (name == "listitem") {
        node.set_name("li");
        // LOG(stringify(node.first_child()));
        contract_assert(stringify(node.first_child()) == "<prepend>&amp;bullet;</prepend>\n");
        node.remove_child(node.first_child());
        return true;
      }
      return true;
    });
    std::map<std::string, std::size_t> nodes;
    xml_recurse(node, [&](pugi::xml_node node) {
      contract_assert(node.name() != std::string_view("ivl_sectiontitle"));
      ++nodes[std::string(node.name())];
      return true;
    });
    for (auto&& [node, count] : nodes) LOG(node, count);
    for (auto child : node) child.print(*html::current_page, "  ");
    texinfo.remove_child(node);
  });

  html::create_page(output_dir / "index.html", [&] {
    html::create_cppref_head("GCC reference");
    html::create_node(
      "body",
      {{"class", "mediawiki ltr sitedir-ltr mw-hide-empty-elt ns-0 ns-subject page-cpp rootpage-cpp skin-cppreference2 "
                 "action-view cpp-navbar"}},
      [&] {
        html::create_cppref_header();
        {
          auto _ = html::create_node_raii("div", {{"id", "cpp-content-base"}});
          auto _ = html::create_node_raii("div", {{"id", "content"}, {"class", "mw-body"}});
          html::create_node("h1", {{"id", "firstHeading"}, {"class", "firstHeading"}}, [&] {
            html::emit_raw("GCC reference");
          });
          auto _ = html::create_node_raii("div", {{"id", "bodyContent"}, {"class", "mw-body-content"}});
          auto _ = html::create_node_raii("div", {{"id", "mw-content-text"}});
          auto _ = html::create_node_raii("div", {{"id", "mw-content-ltr mw-parser-output"}, {"dir", "ltr"}});
          auto _ = html::create_node_raii(
            "table", {
                       {"class", "mainpagetable"},
                       {"cellspacing", "0"},
                       {"style", "/*! width:100%; */ /*! white-space:nowrap; */"},
                     }
          );
          auto _ = html::create_node_raii("tbody");
          html::create_node("tr", {{"class", "row"}}, [&] {
            auto pba = [&](std::string_view url, std::string_view text) {
              html::create_node("p", [&] {
                html::create_node("b", [&] { html::create_node("a", {{"href", url}}, [&] { html::emit_raw(text); }); });
              });
            };
            html::create_node("td", [&] {});
            html::create_node("td", [&] {});
            html::create_node("td", [&] { pba("/reference/gcc/contributors", "Contributors"); });
          });
          html::create_node("tr", {{"class", "row rowbottom"}}, [&] {
            auto _ = html::create_node_raii("td", {{"colspan", "3"}});
            auto _ = html::create_node_raii("a", {{"href", "/reference/gcc/index"}});
            html::emit_raw("Index");
          });
        }
      }
    );
  });

  {
    std::set<std::string_view> bad{
      "macro",           "syncodeindex", "direntry", "titlepage", "node",         "sectiontitle",
      "menuleadingtext", "top",          "appendix", "cindex",    "indexcommand", "columnfraction",
      "columnfractions", "xrefinfofile", "para",     "email",     "emailaddress",
    };
    xml_recurse(doc, [&bad](pugi::xml_node node) {
      contract_assert(!bad.contains(std::string_view(node.name())));
      return true;
    });
  }

  {
    for (auto child : texinfo) LOG(child.name(), child.attribute("ivl_sectiontitle").value(), stringify(child).size());
    // LOG(stringify(texinfo.last_child()));
  }

  {
    LOG(stringify(doc).size());
    LOG(name_count(doc));
    doc.print(std::cout, "  ");
    return 0;
  }
}
