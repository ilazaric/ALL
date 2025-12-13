#include <ivl/fs/fileview>
#include <ivl/io/stlutils>
#include <ivl/logger>
#include <cassert>
#include <iostream>
#include <memory>
#include <ranges>
#include <set>
#include <string>
#include <variant>
#include <vector>

struct Text;
struct Comm;
struct Braces;
struct Elem;

struct Text {
  std::string text;
};
struct Comm {
  std::string comm;
};
struct BracedSection {
  std::vector<Elem> elems;
};
struct Elem {
  std::variant<Text, Comm, BracedSection> var;
};

void skip_whitespace(std::string_view& sv) {
  while (!sv.empty() && std::isspace(sv[0]))
    sv.remove_prefix(1);
}

void consume(std::string_view& sv, char c) {
  assert(!sv.empty() && sv[0] == c);
  sv.remove_prefix(1);
}

char pop(std::string_view& sv) {
  assert(!sv.empty());
  char ret = sv[0];
  sv.remove_prefix(1);
  return ret;
}

Text parse_text(std::string_view& sv) {
  Text text;
  while (!sv.empty() && !std::isspace(sv[0]) && sv[0] != '\\' && sv[0] != '{' && sv[0] != '}') {
    text.text += pop(sv);
  }
  return text;
}

Comm parse_comm(std::string_view& sv) {
  consume(sv, '\\');
  assert(!sv.empty());
  Comm comm;
  if (sv[0] == '{' || sv[0] == '}') {
    comm.comm += pop(sv);
    return comm;
  }
  comm.comm = parse_text(sv).text;
  return comm;
}

std::vector<Elem> parse_latex(std::string_view&);

BracedSection parse_braced_section(std::string_view& sv) {
  BracedSection ret;
  consume(sv, '{');
  ret.elems = parse_latex(sv);
  consume(sv, '}');
  return ret;
}

std::vector<Elem> parse_latex(std::string_view& latex) {
  // LOG(latex);
  std::vector<Elem> ret;
  while (true) {
    skip_whitespace(latex);
    if (latex.empty()) break;
    if (latex[0] == '}') break;
    if (latex[0] == '\\') {
      ret.emplace_back(parse_comm(latex));
    } else if (latex[0] == '{') {
      ret.emplace_back(parse_braced_section(latex));
    } else {
      ret.emplace_back(parse_text(latex));
    }
  }
  return ret;
}

std::vector<std::vector<Elem>> parse_file(ivl::str::NullStringView path, std::string_view section_name) {
  std::string       begin_section = std::string("\\begin{") + std::string(section_name) + "}";
  std::string       end_section   = std::string("\\end{") + std::string(section_name) + "}";
  ivl::fs::FileView fv{path};
  std::string_view  latex((const char*)fv.mapped_region, fv.size());
  // LOG(latex);
  std::vector<std::vector<Elem>> ret;
  while (true) {
    auto beginpos = latex.find(begin_section);
    if (beginpos == std::string_view::npos) break;
    auto endpos = latex.find(end_section);
    // LOG(beginpos, endpos);
    assert(endpos != std::string_view::npos);
    assert(endpos > beginpos);
    auto             beginptr = latex.data() + beginpos + begin_section.size();
    auto             endptr   = latex.data() + endpos;
    std::string_view piece(beginptr, endptr - beginptr);
    ret.push_back(parse_latex(piece));
    latex.remove_prefix(endpos + section_name.size() + 1);
  }
  return ret;
}

struct Definition {
  std::string                    name;
  std::vector<std::vector<Elem>> vars;
};

int main() {
  parse_file("/home/ilazaric/repos/draft/source/grammar.tex", "ncbnf");
  parse_file("/home/ilazaric/repos/draft/source/std-gram.ext", "bnf");
}
