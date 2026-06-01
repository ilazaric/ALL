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
};

int ivl_main(const args& args) {
  contract_assert(!args.output.empty());
  contract_assert(is_directory(args.output));
  auto output_dir = args.output / "reference" / "gcc";
  remove_all(output_dir);
  create_directories(output_dir);

  pugi::xml_document doc;
  if (!doc.load_file(args.file.native().c_str())) return 1;

  LOG(xml::to_string(doc).size());
  LOG(xml::name_count(doc));

  top_level t;

  auto texinfo = doc.first_child();
  contract_assert(texinfo.name() == std::string_view("texinfo"));

  gcc::replace_text_commands(doc);
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

  gcc::purge_node(doc);
  gcc::purge_sectiontitle(doc);
  gcc::purge_menuleadingtext(doc);

  t.top = xml::extract_first_check_name(texinfo, "top");

  {
    auto child = texinfo.last_child();
    contract_assert(child.name() == std::string_view("appendix"));
    t.appendix = xml::to_string(child);
    texinfo.remove_child(child);
  }

  gcc::purge_space_attributes(doc);
  gcc::merge_cindex_indexterm(doc);
  gcc::merge_indexcommand_indexterm(doc);
  gcc::purge_columnfractions(doc);
  gcc::purge_ref(doc);

  {
    auto node = texinfo.last_child();
    contract_assert(node.name() == std::string_view("unnumbered"));
    contract_assert(node.attribute("ivl_sectiontitle").value() == std::string_view("Contributors to GCC"));
  }

  gcc::transform_email(doc);
  gcc::transform_url(doc);

  for (auto name : {"option", "samp", "file", "command"}) {
    xml::recurse_name(doc, name, [&](pugi::xml_node node) {
      node.set_name("code");
      node.prepend_attribute("ivl_kind").set_value(name);
      return true;
    });
  }

  gcc::transform_accent(doc);

  auto page_last = [&](pugi::xml_node node) {
    xml::purge_name(node, "beforefirstitem");
    xml::purge_name(node, "itemprepend");
    xml::purge_name(node, "ivl_cindex_indexterm");
    xml::purge_name(node, "ignore");
    xml::purge_name(node, "noindent");
    gcc::transform_divlike(node);
    // auto node = texinfo.last_child();
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
      if (name == "heading") {
        node.set_name("h3");
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
    nodes.push_back(page_last(texinfo.last_child()));
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
