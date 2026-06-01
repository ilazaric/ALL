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
    gcc::transform_acronym(node);
    gcc::transform_heading(node);
    gcc::transform_listlike(node);
    // auto node = texinfo.last_child();
    contract_assert(node.name() == std::string_view("unnumbered") || node.name() == std::string_view("chapter"));
    std::string_view sectiontitle = node.attribute("ivl_sectiontitle").value();
    contract_assert(sectiontitle.size());
    std::string_view nodename = node.attribute("ivl_nodename").value();
    LOG(sectiontitle, nodename);
    contract_assert(nodename.size());
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
