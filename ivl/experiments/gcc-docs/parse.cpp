#include <ivl/logger>
#include <ivl/utility>
#include "gcc_utils"
#include "html_gen"
#include "xml_utils"
#include <pugixml/pugixml.hpp>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <print>
#include <ranges>
#include <set>
#include <source_location>

struct args {
  std::filesystem::path file;
  // std::size_t depth;
  std::filesystem::path output;
};

static_assert(sizeof(pugi::xml_node) == 8);

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

  LOG(xml::to_string(doc).size());
  LOG(xml::name_count(doc));

  top_level t;

  auto texinfo = doc.first_child();
  contract_assert(texinfo.name() == std::string_view("texinfo"));

  gcc::purge_duds(doc);

  xml::recurse(texinfo, [&](pugi::xml_node node) {
    if (node.name() == std::string_view("macro")) {
      t.macros.push_back(xml::to_string(node));
      node.parent().remove_child(node);
    } else if (node.name() == std::string_view("syncodeindex")) {
      t.syncodeindexs.push_back(xml::to_string(node));
      node.parent().remove_child(node);
    }
    return false;
  });

  t.direntry = xml::extract_first_check_name(texinfo, "direntry");
  t.titlepage = xml::extract_first_check_name(texinfo, "titlepage");

  xml::recurse_name(doc, "para", [&](pugi::xml_node node) {
    node.set_name("p");
    return true;
  });

  // email.emailaddress -> emailaddress
  xml::recurse_name(doc, "email", [&](pugi::xml_node node) {
    contract_assert(std::ranges::distance(node) == 1);
    contract_assert(std::ranges::distance(node.attributes()) == 0);
    contract_assert(node.first_child().name() == std::string_view("emailaddress"));
    auto child = node.first_child();
    auto parent = node.parent();
    parent.insert_move_before(child, node);
    parent.remove_child(node);
    return false;
  });

  xml::recurse_name(doc, "node", [&](pugi::xml_node node) {
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

  xml::recurse_name(doc, "sectiontitle", [](pugi::xml_node node) {
    auto parent = node.parent();
    contract_assert(node == parent.first_child());
    bool check = parent.prepend_attribute("ivl_sectiontitle").set_value(node.text().get());
    contract_assert(check);
    parent.remove_child(node);
    return false;
  });

  xml::recurse_name(doc, "menuleadingtext", [](pugi::xml_node node) {
    contract_assert(xml::to_string(node) == "<menuleadingtext>* </menuleadingtext>\n");
    node.parent().remove_child(node);
    return false;
  });

  t.top = xml::extract_first_check_name(texinfo, "top");

  {
    auto child = texinfo.last_child();
    contract_assert(child.name() == std::string_view("appendix"));
    t.appendix = xml::to_string(child);
    texinfo.remove_child(child);
  }

  {
    auto child = texinfo.first_child();
    contract_assert(child.name() == std::string_view("chapter"));
    contract_assert(
      child.attribute("ivl_sectiontitle").value() == std::string_view("Programming Languages Supported by GCC")
    );
    t.supported_languages = xml::to_string(child);
    texinfo.remove_child(child);
  }

  {
    auto child = texinfo.first_child();
    contract_assert(child.name() == std::string_view("chapter"));
    contract_assert(
      child.attribute("ivl_sectiontitle").value() == std::string_view("Language Standards Supported by GCC")
    );
    t.supported_standards = xml::to_string(child);
    texinfo.remove_child(child);
  }

  xml::recurse(doc, [](pugi::xml_node node) {
    auto a = node.attribute("spaces");
    if (a) node.remove_attribute(a);
    return true;
  });

  gcc::merge_cindex_indexterm(doc);
  gcc::merge_indexcommand_indexterm(doc);

  xml::recurse(doc, [&](pugi::xml_node node) {
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

  xml::recurse_name(doc, "columnfractions", [&](pugi::xml_node node) {
    auto parent = node.parent();
    contract_assert(parent.name() == std::string_view("multitable"));
    auto at = parent.append_attribute("ivl_cf_line");
    at.set_value(node.attribute("line").value());
    parent.remove_child(node);
    return false;
  });

  xml::recurse(doc, [&](pugi::xml_node node) {
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

  xml::recurse(doc, [](pugi::xml_node node) {
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
      t.opts.intro.push_back(xml::to_string(child));
      child = child.next_sibling();
    }
    contract_assert(child.name() == std::string_view("menu"));
    t.opts.menu = xml::to_string(child);
    child = child.next_sibling();
    while (child) {
      contract_assert(child.name() == std::string_view("section"));
      t.opts.sections.push_back(xml::to_string(child));
      child = child.next_sibling();
    }
    texinfo.remove_child(chapter);
  }

  {
    auto node = texinfo.last_child();
    contract_assert(node.name() == std::string_view("unnumbered"));
    contract_assert(node.attribute("ivl_sectiontitle").value() == std::string_view("Contributors to GCC"));
    std::map<std::string, std::size_t> names;
    xml::recurse(node, [&](pugi::xml_node node) {
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

  xml::recurse_name(doc, "", [&](pugi::xml_node node) {
    std::string value = node.value();
    replace_cmds(value);
    node.set_value(value);
    return false;
  });

  xml::recurse_name(doc, "emailaddress", [&](pugi::xml_node node) {
    xml::assert_wraps_text(node);
    node.set_name("a");
    node.append_attribute("class").set_value("email");
    std::string email = node.text().get();
    node.append_attribute("href").set_value("mailto:" + email);
    return false;
  });

  xml::recurse(doc, [&](pugi::xml_node node) {
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

  for (auto name : {"option", "samp", "file", "command"}) {
    xml::recurse_name(doc, name, [&](pugi::xml_node node) {
      node.set_name("code");
      node.prepend_attribute("ivl_kind").set_value(name);
      return true;
    });
  }

  xml::recurse_name(doc, "accent", [](pugi::xml_node node) {
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

  auto page_last = [&] {
    auto node = texinfo.last_child();
    contract_assert(node.name() == std::string_view("unnumbered") || node.name() == std::string_view("chapter"));
    std::string_view sectiontitle = node.attribute("ivl_sectiontitle").value();
    contract_assert(sectiontitle.size());
    std::string_view nodename = node.attribute("ivl_nodename").value();
    LOG(sectiontitle, nodename);
    contract_assert(nodename.size());
    // contract_assert(node.first_child().name() == std::string_view("ivl_cindex_indexterm"));
    // node.remove_child(node.first_child());
    xml::recurse(node, [&](pugi::xml_node node) {
      std::string_view name = node.name();
      if (
        name == "beforefirstitem" || name == "itemprepend" || name == "ivl_cindex_indexterm" || name == "ignore" ||
        name == "noindent"
      ) {
        node.parent().remove_child(node);
        return false;
      }
      if (name == "heading") {
        node.set_name("h3");
        return true;
      }
      if (name == "center") {
        xml::assert_wraps_text(node);
        node.set_name("div");
        node.append_attribute("class").set_value("center");
        return false;
      }
      if (name == "display") {
        contract_assert(std::ranges::distance(node.attributes()) == 0);
        node.set_name("div");
        node.append_attribute("class").set_value("display");
        return true;
      }
      if (name == "smallexample") {
        contract_assert(std::ranges::distance(node.attributes()) == 0);
        node.set_name("div");
        node.append_attribute("class").set_value("example smallexample");
        return true;
      }
      if (name == "acronym") {
        contract_assert(std::ranges::distance(node) == 1);
        contract_assert(std::ranges::distance(node.attributes()) == 0);
        xml::assert_wraps_text(node.first_child());
        contract_assert(node.first_child().name() == std::string_view("acronymword"));
        node.append_move(node.first_child().first_child());
        node.remove_child(node.first_child());
        node.set_name("abbr");
        node.append_attribute("class").set_value("acronym");
      }
      contract_assert(name != std::string_view("acronymword"));
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
    std::map<std::string, std::size_t> nodes;
    xml::recurse(node, [&](pugi::xml_node node) {
      contract_assert(node.name() != std::string_view("ivl_sectiontitle"));
      ++nodes[std::string(node.name())];
      return true;
    });
    for (auto&& [node, count] : nodes) LOG(node, count);
    html::create_cppref_page(output_dir / nodename / "index.html", sectiontitle, [&] {
      for (auto child : node) child.print(html::current_page(), "  ");
    });
    // texinfo.remove_child(node);
    return std::pair<std::string, std::string>(nodename, sectiontitle);
  };

  std::vector<std::pair<std::string, std::string>> nodes;
  for (int i = 0; i < 8; ++i) {
    nodes.push_back(page_last());
    texinfo.remove_child(texinfo.last_child());
  }
  std::ranges::reverse(nodes);

  html::create_cppref_page(output_dir / "index.html", "GCC reference", [&] {
    auto _ = html::create_node_raii(
      "table", {
                 {"class", "mainpagetable"},
                 {"cellspacing", "0"},
                 {"style", "width:100%; white-space:nowrap;"},
               }
    );
    auto _ = html::create_node_raii("tbody");
    html::create_node("tr", {{"class", "row"}}, [&] {
      auto pba = [&](std::string_view url, std::string_view text) {
        html::create_node("p", [&] {
          html::create_node("b", [&] { html::create_node("a", {{"href", url}}, [&] { html::emit_raw(text); }); });
        });
      };
      html::create_node("td", [&] {
        for (std::size_t i = 0; i < nodes.size() / 3; ++i)
          pba(std::string("/reference/gcc/") + nodes[i].first, nodes[i].second);
      });
      html::create_node("td", [&] {
        for (std::size_t i = nodes.size() / 3; i < nodes.size() * 2 / 3; ++i)
          pba(std::string("/reference/gcc/") + nodes[i].first, nodes[i].second);
      });
      html::create_node("td", [&] {
        for (std::size_t i = nodes.size() * 2 / 3; i < nodes.size(); ++i)
          pba(std::string("/reference/gcc/") + nodes[i].first, nodes[i].second);
        // pba("/reference/gcc/Contributors", "Contributors");
      });
    });
    html::create_node("tr", {{"class", "row rowbottom"}}, [&] {
      auto _ = html::create_node_raii("td", {{"colspan", "3"}});
      auto _ = html::create_node_raii("a", {{"href", "/reference/gcc/index"}});
      html::emit_raw("Index");
    });
  });

  {
    std::set<std::string_view> bad{
      "macro",           "syncodeindex", "direntry", "titlepage", "node",         "sectiontitle",
      "menuleadingtext", "top",          "appendix", "cindex",    "indexcommand", "columnfraction",
      "columnfractions", "xrefinfofile", "para",     "email",     "emailaddress",
    };
    xml::recurse(doc, [&bad](pugi::xml_node node) {
      contract_assert(!bad.contains(std::string_view(node.name())));
      return true;
    });
  }

  {
    for (auto child : texinfo)
      LOG(child.name(), child.attribute("ivl_sectiontitle").value(), xml::to_string(child).size());
    // LOG(xml::to_string(texinfo.last_child()));
  }

  {
    LOG(xml::to_string(doc).size());
    LOG(xml::name_count(doc));
    doc.print(std::cout, "  ");
    return 0;
  }
}
