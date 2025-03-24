#include <cassert>
#include <vector>
#include <memory>
#include <variant>
#include <ivl/io/stlutils.hpp>
#include <ivl/fs/fileview.hpp>
#include <ivl/logger>
#include <ranges>
#include <string>
#include <set>

struct Text;
struct Comm;
struct Braces;
struct Elem;

struct Text {std::string text;};
struct Comm {std::string comm;};
struct BracedSection {std::vector<Elem> elems;};
struct Elem {std::variant<Text, Comm, BracedSection> var;};

void skip_whitespace(std::string_view& sv){
  while (!sv.empty() && std::isspace(sv[0])) sv.remove_prefix(1);
}

void consume(std::string_view& sv, char c){
  assert(!sv.empty() && sv[0] == c);
  sv.remove_prefix(1);
}

char pop(std::string_view& sv){
  assert(!sv.empty());
  char ret = sv[0];
  sv.remove_prefix(1);
  return ret;
}

Text parse_text(std::string_view& sv){
  Text text;
  while (!sv.empty() && !std::isspace(sv[0]) && sv[0] != '\\' && sv[0] != '{' && sv[0] != '}'){
    text.text += pop(sv);
  }
  return text;
}

Comm parse_comm(std::string_view& sv){
  consume(sv, '\\');
  assert(!sv.empty());
  Comm comm;
  if (sv[0] == '{' || sv[0] == '}'){
    comm.comm += pop(sv);
    return comm;
  }
  comm.comm = parse_text(sv).text;
  return comm;
}

std::vector<Elem> parse_latex(std::string_view&);

BracedSection parse_braced_section(std::string_view& sv){
  BracedSection ret;
  consume(sv, '{');
  ret.elems = parse_latex(sv);
  consume(sv, '}');
  return ret;
}

std::vector<Elem> parse_latex(std::string_view latex){
  std::vector<Elem> ret;
  while (true){
    skip_whitespace(latex);
    if (latex.empty()) break;
    if (latex[0] == '}') break;
    if (latex[0] == '\\'){
      ret.emplace_back(parse_comm(latex));
    } else if (latex[0] == '{'){
      ret.emplace_back(parse_braced_section(latex));
    } else {
      ret.emplace_back(parse_text(latex));
    }
  }
  return ret;
}

constexpr std::string_view BEGIN = "\\begin{bnf}";
constexpr std::string_view END = "\\end{bnf}";

struct Node;
struct Text;
struct Comm;
struct CommArg;
struct Node {std::vector<std::variant<Text, Comm, CommArg>> elems;};
struct Text {std::string text;};
struct Comm {std::string comm;};
struct CommArg {std::string comm; Node arg;};

std::ostream& operator<<(std::ostream& out, const Node& el);
std::ostream& operator<<(std::ostream& out, const Text& el){return out << '"' << el.text << '"';}
std::ostream& operator<<(std::ostream& out, const Comm& el){return out << '\\' << el.comm;}
std::ostream& operator<<(std::ostream& out, const CommArg& el){return out << '\\' << el.comm << "{ " << el.arg << '}';}
std::ostream& operator<<(std::ostream& out, const Node& node){
  for (const auto& var : node.elems)
    std::visit([&](const auto& el){out << el << " ";}, var);
  return out;
}

void rmws(std::string_view& sv){
  while (!sv.empty() && std::isspace(sv[0])) sv.remove_prefix(1);
}

std::string_view consumestr(std::string_view& sv){
  if (sv.empty()) return {};
  std::size_t idx = 0;
  while (idx < sv.size() && !std::isspace(sv[idx]) && sv[idx] != '\\' && sv[idx] != '{' && sv[idx] != '}') ++idx;
  auto ret = sv.substr(0, idx);
  sv.remove_prefix(idx);
  return ret;
}

Node parse(std::string_view& sv){
  LOG("parse enter");
  Node ret;
  while (true){
    LOG(sv.substr(0, 100));
    rmws(sv);
    if (sv.empty()) break;
    if (sv[0] == '}'){
      sv.remove_prefix(1);
      break;
    }

    if (sv[0] == '\\'){ // comm or commarg
      sv.remove_prefix(1);
      auto name = consumestr(sv);
      rmws(sv);
      if (!sv.empty() && sv[0] == '{'){ // commarg
        sv.remove_prefix(1);
        ret.elems.emplace_back(CommArg(std::string(name), parse(sv)));
      } else { // comm
        ret.elems.emplace_back(Comm(std::string(name)));
      }
    } else { // text
      ret.elems.emplace_back(Text(std::string(consumestr(sv))));
    }
  }
  LOG("parse exit");
  return ret;
}

struct Def {
  std::string name;
  std::vector<Node> vars;
};

Def normalize(Node&& node){
  LOG("normalize enter");
  Def ret;
  { // name
    auto it = std::ranges::find_if(node.elems, [](auto&& el){
      auto ptr = std::get_if<CommArg>(&el);
      if (ptr == nullptr) return false;
      if (ptr->comm != "nontermdef") return false;
      return true;
    });
    assert(it != node.elems.end());
    auto& bla = (std::get<CommArg>(*it));
    assert(bla.arg.elems.size() == 1);
    auto& truc = bla.arg.elems[0];
    ret.name = std::get<Text>(truc).text;
  }
  { // vars
    auto br = [](auto&& var){
      auto ptr = std::get_if<Comm>(&var);
      if (ptr == nullptr) return false;
      return ptr->comm == "br";
    };
    for (auto it = std::ranges::find_if(node.elems, br); it != node.elems.end();){
      ++it;
      ret.vars.emplace_back();
      auto& curr = ret.vars.back();
      for (auto it2 = std::ranges::find_if(it, node.elems.end(), br); it != it2; ++it){
        curr.elems.push_back(*it);
      }
    }
  }
  LOG("normalize exit");
  return ret;
}

std::string camel_case(const std::string& s){
  std::string out;
  for (auto&& e : s | std::views::split('-')){
    std::string_view bla(e);
    out += std::toupper(bla[0]);
    out += bla.substr(1);
  }
  return out;
}

void print_terminal_components(auto&& el, bool& first){
  std::visit([&]<typename T>(const T& x){
      if constexpr (std::same_as<T, Text>){
        if (!first) std::cout << ", "; first = false;
        std::cout << "\"" << x.text << "\"";
      } else if constexpr (std::same_as<T, Comm>){
        if (!first) std::cout << ", "; first = false;
        if (x.comm == "#"){
          std::cout << "\"" << x.comm << "\"";
        } else {
          std::cerr << x << std::endl;
          assert(false);
        }
      } else if constexpr (std::same_as<T, CommArg>){
        if (x.comm == "xname"){
          assert(x.arg.elems.size() == 1);
          auto y = std::get_if<Text>(&x.arg.elems[0]);
          assert(y);
          if (!first) std::cout << ", "; first = false;
          std::cout << "\"__" << y->text << "\"";
        } else if (x.comm == "mname"){
          assert(x.arg.elems.size() == 1);
          auto y = std::get_if<Text>(&x.arg.elems[0]);
          assert(y);
          if (!first) std::cout << ", "; first = false;
          std::cout << "\"__" << y->text << "__\"";
        } else {
          LOG(x);
          assert(false);
        }
      } else static_assert(false);
    }, el);
}

void print_single(auto&& el){
  std::visit([]<typename T>(const T& x){
      if constexpr (std::same_as<T, Text>){
        std::cout << camel_case(x.text);
      } else if constexpr (std::same_as<T, Comm>){
        assert(false);
      } else if constexpr (std::same_as<T, CommArg>){
        if (x.comm == "opt"){
          assert(x.arg.elems.size() == 1);
          std::cout << "std::optional<";
          print_single(x.arg.elems[0]);
          std::cout << ">";
        } else if (x.comm == "terminal"){
          std::cout << "Terminal<";
          bool f = true;
          for (auto&& y : x.arg.elems)
            print_terminal_components(y, f);
          std::cout << ">";
        } else if (x.comm == "keyword"){
          assert(x.arg.elems.size() == 1);
          auto y = std::get_if<Text>(&x.arg.elems[0]);
          assert(y);
          std::cout << "Keyword<\"" << y->text << "\">";
        } else std::cout << x;
      } else static_assert(false);
    }, el);
}

std::vector<Def> parse_file(ivl::str::NullStringView path){
  ivl::fs::FileView fv(path);
  auto span = fv.get_remaining();
  std::string_view sv{(const char*)span.data(), span.size()};

  std::vector<Def> defs;

  while (true){
    auto begin = sv.find(BEGIN);
    if (begin == sv.npos) break;
    begin += BEGIN.size();
    auto end = sv.find(END, begin);
    assert(end != sv.npos);

    auto elem = sv.substr(begin, end - begin);
    defs.emplace_back(normalize(parse(elem)));

    sv = sv.substr(end + END.size());
  }

  return defs;
}

int main(){
  std::vector<Def> defs = parse_file("/home/ilazaric/repos/draft/source/grammar.tex");
  std::vector<Def> defs2 = parse_file("/home/ilazaric/repos/draft/source/std-gram.ext");
  LOG(defs.size());
  LOG(defs2.size());
  defs.insert(defs.end(), defs2.begin(), defs2.end());

  std::cout << "#include <memory>" << std::endl;
  std::cout << "#include <variant>" << std::endl;
  std::cout << "#include <tuple>" << std::endl;
  std::cout << std::endl;

  for (auto&& def : defs){
    std::cout << "struct " << camel_case(def.name) << ";" << std::endl;
  }

  std::cout << std::endl;

  std::set<std::string> skip{"lparen", "new-line", "h-preprocessing-token"};

  for (auto&& def : defs){
    if (skip.contains(def.name)) continue;
    std::cout << "struct " << camel_case(def.name) << " {" << std::endl;
    std::cout << "  std::unique_ptr<" << std::endl;
    std::cout << "    " << "std::variant<";
    bool first = true;
    for (auto&& var : def.vars){
      if (!first) std::cout << ",";
      first = false;
      std::cout << std::endl;
      std::cout << "      " << "std::tuple<";
      bool first = true;
      for (auto&& el : var.elems){
        if (!first) std::cout << ",";
        first = false;
        std::cout << std::endl;
        std::cout << "        ";
        print_single(el);
      }
      std::cout << std::endl;
      std::cout << "      " << ">";
    }
    std::cout << std::endl;
    std::cout << "    >" << std::endl;
    std::cout << "  > data;" << std::endl;
    std::cout << "};" << std::endl << std::endl;
  }
  
}
