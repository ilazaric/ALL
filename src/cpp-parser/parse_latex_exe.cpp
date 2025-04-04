#include "parse_latex.hpp"

#include <set>
#include <map>
#include <ranges>
#include <boost/algorithm/string/split.hpp>
#include <ivl/io/stlutils.hpp>

using namespace ivl;
using namespace cppp;

template<typename... Ts>
struct Overload : Ts... {
  using Ts::operator()...;
};
template<typename... Ts>
Overload(Ts...) -> Overload<Ts...>;

template<typename A, typename B>
bool eq(const A& a, const B& b){
  if constexpr (std::is_same_v<A, LatexElem>){
    return std::visit([&](auto&& x){return eq(x, b);},
                      a.kind);
  } else if constexpr (std::is_same_v<B, LatexElem>){
    return std::visit([&](auto&& x){return eq(x, a);},
                      b.kind);
  } else if constexpr (!std::is_same_v<A, B>){
    return false;
  } else {
    return a == b;
  }
}

struct DefinitionElem;

struct DefinitionText {
  std::string text;
};

struct DefinitionCommand {
  std::string name;
  std::vector<DefinitionElem> contents;
};

struct DefinitionElem {
  std::variant<DefinitionText, DefinitionCommand> kind;
};

struct DefinitionRow {
  std::vector<DefinitionElem> conjunction;
};

struct Definition {
  std::string term;
  bool error;
  bool one_of;
  std::vector<DefinitionRow> disjunction;
};

const auto dump_latex = []<typename T>(this const auto& self, const T& entity){
  if constexpr (std::is_same_v<T, LatexText>){
    std::cerr << '"' << entity.text << '"';
  } else if constexpr (std::is_same_v<T, LatexCommand>){
    std::cerr << "\\" << entity.name;
    for (auto& arg : entity.args) self(arg);
  } else if constexpr (std::is_same_v<T, LatexArg>){
    std::cerr << (entity.kind == LatexArg::Kind::CURLY ? '{' : '[');
    self(entity.contents);
    std::cerr << (entity.kind == LatexArg::Kind::CURLY ? '}' : ']');
  } else if constexpr (std::is_same_v<T, Latex>){
    bool first = true;
    for (auto& el : entity.elems){
      if (!first) std::cerr << " ";
      self(el);
      first = false;
    }
  } else if constexpr (std::is_same_v<T, LatexElem>){
    std::visit(self, entity.kind);
  } else if constexpr(std::is_same_v<T, LatexEnvironment>){
    std::cerr << entity.name << " ";
    self(entity.begin_command);
    std::cerr << " ";
    self(entity.contents);
    std::cerr << " ";
    self(entity.end_command);
  } else {
    static_assert(false);
  }
 };

std::vector<std::string> split(std::string_view sv){
  // LOG(sv);
  std::vector<std::string> res;
  boost::algorithm::split(res, sv, ::isspace, boost::token_compress_on);
  // for (auto&& e : res) LOG(e);
  size_t b = 0;
  for (size_t a = 0; a != res.size(); ++a)
    if (!res[a].empty()){
      // LOG(b, a);
      if (a != b) res[b] = std::move(res[a]);
      ++b;
    }
  // LOG(b);
  res.erase(res.begin() + b, res.end());
  // for (auto&& e : res) LOG(e);
  return res;
}

void fillup(std::vector<DefinitionElem>& res, const LatexElem& elem){
  std::visit([&]<typename T>(const T& x){
      if constexpr (std::is_same_v<T, LatexText>){
        auto pieces = split(x.text);
        for (auto&& piece : pieces)
          res.emplace_back(DefinitionText{std::move(piece)});
      } else { // LatexCommand
        res.emplace_back(DefinitionCommand{x.name});
        auto& dc = std::get<DefinitionCommand>(res.back().kind);
        if (x.name != "unicode") assert(x.args.size() <= 1);
        else assert(x.args.size() == 2);
        for (auto&& arg : x.args | std::views::take(1)){
          assert(arg.kind == LatexArg::Kind::CURLY);
          for (auto&& el : arg.contents.elems)
            fillup(dc.contents, el);
        }
      }
    }, elem.kind);
}

Definition parse_bnf(const LatexEnvironment& env){
  const auto& contents = env.contents.elems;
  size_t idx = 0;

  auto skip_whitespace_text = [&]{
    while (idx != contents.size()){
      auto text = std::get_if<LatexText>(&contents[idx].kind);
      if (text == nullptr) return;
      if (std::ranges::all_of(text->text, ::isspace)){
        ++idx;
        continue;
      } else return;
    }
  };

  skip_whitespace_text();
  if (idx != contents.size()){
    auto comm = std::get_if<LatexCommand>(&contents[idx].kind);
    if (comm != nullptr && comm->name == "microtypesetup") ++idx;
  }
  skip_whitespace_text();
  if (idx != contents.size() && eq(contents[idx], LatexCommand{"obeyspaces"})) ++idx;
  
  const auto term = ({
      skip_whitespace_text();
      if (idx == contents.size()) return {"???", true};
      const auto nontermdef = std::get_if<LatexCommand>(&contents[idx].kind);
      if (nontermdef == nullptr) return {"???", true};
      if (nontermdef->name != "nontermdef") return {"???", true};
      if (nontermdef->args.size() != 1) return {"???", true};
      if (nontermdef->args[0].kind != LatexArg::Kind::CURLY) return {"???", true};
      if (nontermdef->args[0].contents.elems.size() != 1) return {"???", true};
      const auto nontermdef_term = std::get_if<LatexText>(&nontermdef->args[0].contents.elems[0].kind);
      if (nontermdef_term == nullptr) return {"???", true};
      ++idx;
      nontermdef_term->text;
  });

  // if (term == "operator"){
  //   LOG(term);
  //   dump_latex(env);
  //   std::cerr << std::endl;
  //   exit(1);
  // }

  skip_whitespace_text();
  bool one_of = false;
  if (idx != contents.size() && eq(contents[idx], LatexCommand{"textnormal", {LatexArg{LatexArg::Kind::CURLY, Latex{{LatexElem{LatexText{"one of"}}}}}}})){
    one_of = true;
    ++idx;
  }

  std::vector<DefinitionRow> rows;
  while (true){
    skip_whitespace_text();
    if (idx == contents.size()) break;
    if (!eq(contents[idx], LatexCommand{"br"})) return {term, true};
    ++idx;
    rows.emplace_back();
    auto& row = rows.back();
    while (true){
      skip_whitespace_text();
      if (idx == contents.size() || eq(contents[idx], LatexCommand{"br"})) break;
      fillup(row.conjunction, contents[idx]);
      ++idx;
    }
  }
  return {term, false, one_of, rows};
}

void dump(const DefinitionElem& elem){
  visit([]<typename T>(const T& x){
      if constexpr (std::is_same_v<T, DefinitionText>){
        std::cout << "'" << x.text << "'";
      } else {
        std::cout << x.name << "{";
        for (auto&& e : x.contents){
          std::cout << " ";
          dump(e);
        }
        std::cout << " }";
      }
    }, elem.kind);
}

void dump(const DefinitionRow& row){
  for (auto&& el : row.conjunction){
    std::cout << " ";
    dump(el);
  }
  std::cout << std::endl;
}

void dump(const Definition& def){
  std::cout << def.term << " ::=" << std::endl;
  if (def.error){
    std::cout << "  *** ERROR ***" << std::endl;
    return;
  }
  if (def.one_of){
    std::cout << "  -- ONE OF --" << std::endl;
  }
  for (auto&& row : def.disjunction){
    std::cout << "  |";
    dump(row);
  }
}

std::vector<std::string> find_keywords(){
  std::filesystem::path file{"/home/ilazaric/repos/draft/source/lex.tex"};
  Latex latex = parse_latex_file(file);
  auto envs = find_environments(latex);
  std::vector<LatexEnvironment> matches;
  for (auto& env : envs){
    bool good = false;
    for (auto& arg : env.begin_command.args){
      if (arg.kind != LatexArg::Kind::CURLY) continue;
      for (auto& el : arg.contents.elems)
        if (eq(el, LatexText{"Keywords"}) ||
            eq(el, LatexText{"Alternative representations"}) ||
            eq(el, LatexText{"Identifiers with special meaning"}))
          good = true;
    }
    if (good) matches.emplace_back(std::move(env));
  }
  assert(matches.size() == 3);
  std::vector<std::string> keywords;
  for (auto& env : matches){
  for (auto& el : env.contents.elems){
    auto command = std::get_if<LatexCommand>(&el.kind);
    if (command == nullptr || command->name != "keyword") continue;
    assert(command->args.size() == 1);
    auto& arg = command->args[0];
    assert(arg.kind == LatexArg::Kind::CURLY);
    assert(arg.contents.elems.size() == 1);
    auto& text = std::get<LatexText>(arg.contents.elems[0].kind);
    keywords.emplace_back(std::move(text.text));
  }
  }
  return keywords;
}

std::set<std::string> find_keywords2(){
  std::filesystem::path dir("/home/ilazaric/repos/my-draft/source");
  std::vector<std::filesystem::path> files;

  for (const auto& dir_entry :
         std::filesystem::recursive_directory_iterator(dir)){
    if (!dir_entry.is_regular_file()) continue;
    auto& path = dir_entry.path();
    if (path.extension() != ".tex") continue;
    if (path.stem() == "macros") continue;
    files.push_back(path);
  }

  std::set<std::string> keywords;
  const auto searcher = [&]<typename T>(this const auto& self, const T& entity){
    if constexpr (std::is_same_v<T, LatexText>){}
    else if constexpr (std::is_same_v<T, LatexElem>){
      std::visit(self, entity.kind);
    } else if constexpr (std::is_same_v<T, Latex>){
      for (auto& el : entity.elems) self(el);
    } else if constexpr (std::is_same_v<T, LatexCommand>){
      if (entity.name == "keyword"){
        // 2 is from misparsing \keyword{new}[]
        // LOG(entity.args.size());
        if (entity.args.size() != 1){
          dump_latex(entity);
          std::cerr << std::endl;
        }
        assert(entity.args.size() == 1 || entity.args.size() == 2);
        assert(entity.args[0].contents.elems.size() == 1);
        auto& text = std::get<LatexText>(entity.args[0].contents.elems[0].kind);
        keywords.insert(text.text);
        return;
      }
      for (auto& arg : entity.args) self(arg.contents);
    } else {
      static_assert(false);
    }
  };

  for (auto& file : files){
    // LOG(file);
    Latex latex = parse_latex_file(file);
    searcher(latex);
  }

  // for (auto kw : keywords) LOG(kw);
  return keywords;
}

size_t count_terminals(const Latex& latex, const std::string& text);

size_t count_terminals(const LatexElem& elem, const std::string& kw){
  auto command = std::get_if<LatexCommand>(&elem.kind);
  if (command == nullptr) return 0;
  size_t count = 0;
  for (auto& arg : command->args) count += count_terminals(arg.contents, kw);
  if (command->name != "terminal"
      // && command->name != "tcode"
      ) return count;
  if (command->args.size() != 1){
    LOG(command->args.size());
    dump_latex(*command);
    std::cerr << std::endl;
  }
  assert(command->args.size() == 1);
  for (auto& el : command->args[0].contents.elems){
    auto text = std::get_if<LatexText>(&el.kind);
    if (text == nullptr) continue;
    auto words = split(text->text);
    for (auto& word : words) if (word == kw) ++count;
  }
  return count;
}

size_t count_terminals(const Latex& latex, const std::string& text){
  size_t count = 0;
  for (auto& el : latex.elems) count += count_terminals(el, text);
  return count;
}

void find_terminal_keywords(){
  auto keywords = find_keywords();
  std::filesystem::path dir("/home/ilazaric/repos/my-draft/source");
  std::vector<std::filesystem::path> files{
    "/home/ilazaric/repos/my-draft/source/std-gram.ext"
  };

  // for (const auto& dir_entry :
  //        std::filesystem::recursive_directory_iterator(dir)){
  //   if (!dir_entry.is_regular_file()) continue;
  //   auto& path = dir_entry.path();
  //   if (path.extension() != ".tex") continue;
  //   if (path.stem() == "macros") continue;
  //   files.push_back(path);
  // }

  LOG(files.size());
  for (auto& file : files){
    LOG(file);
    std::map<std::string, size_t> counts;
    auto latex = parse_latex_file(file);
    for (auto& kw : keywords) counts[kw] = count_terminals(latex, kw);
    for (auto& [kw, cnt] : counts) if (cnt) LOG(kw, cnt);
  }
}

void normalize_one_of(Definition& def){
  if (!def.one_of) return;
  if (def.error){
    dump(def);
    assert(false);
  }

  std::vector<DefinitionRow> new_rows;
  for (auto& dis : def.disjunction){
    for (auto& con : dis.conjunction){
      auto& comm = std::get<DefinitionCommand>(con.kind);
      assert(comm.name == "terminal");
      for (auto& el : comm.contents){
        auto& text = std::get<DefinitionText>(el.kind);
        new_rows.emplace_back(DefinitionRow{{DefinitionElem{DefinitionCommand{"terminal", {DefinitionElem{text}}}}}});
      }
    }
  }

  def.one_of = false;
  def.disjunction = new_rows;
}

const auto kill_caret = []<typename T>(this const auto& self, T& entity){
  if constexpr (std::is_same_v<T, Latex>){
    for (auto& el : entity.elems) self(el);
  } else if constexpr (std::is_same_v<T, LatexElem>){
    auto comm = std::get_if<LatexCommand>(&entity.kind);
    if (comm && comm->name == "caret"){
      assert(comm->args.size() <= 1);
      if (comm->args.size() == 1)
        assert(comm->args[0].contents.elems.empty());
      entity = LatexElem{LatexText{"^"}};
    } else {
      std::visit(self, entity.kind);
    }
  } else if constexpr (std::is_same_v<T, LatexText>){
    // nothing
  } else if constexpr (std::is_same_v<T, LatexCommand>){
    for (auto& arg : entity.args) self(arg.contents);
  } else {
    static_assert(false);
  }
};

std::set<char> escapes{'%', '&', '~', '{', '}', '#'};
const auto kill_escapes = Overload{
  [](this auto&& self, Latex& latex){for (auto& el : latex.elems) self(el);},
  [](this auto&& self, LatexElem& elem){
    auto command = std::get_if<LatexCommand>(&elem.kind);
    if (command && command->name.size() == 1 && escapes.contains(command->name[0])){
      assert(command->args.empty());
      elem = LatexElem{LatexText{command->name}};
    } else {
      std::visit(self, elem.kind);
    }
  },
  [](this auto&& self, LatexText&){},
  [](this auto&& self, LatexCommand& command){for (auto& arg : command.args) self(arg);},
  [](this auto&& self, LatexArg& arg){self(arg.contents);}
};

const auto kill_rlap = Overload{
  [](this auto&& self, Latex& latex){for (auto& el : latex.elems) self(el);},
  [](this auto&& self, LatexElem& elem) -> void {
    auto command = std::get_if<LatexCommand>(&elem.kind);
    if (command && command->name == "rlap"){
      assert(command->args.size() == 1);
      assert(command->args[0].contents.elems.size() == 1);
      elem = command->args[0].contents.elems[0];
      self(elem);
    } else {
      std::visit(self, elem.kind);
    }
  },
  [](this auto&& self, LatexText&){},
  [](this auto&& self, LatexCommand& command){for (auto& arg : command.args) self(arg);},
  [](this auto&& self, LatexArg& arg){self(arg.contents);}
};

const auto kill_textbackslash = Overload{
  [](this auto&& self, Latex& latex){for (auto& el : latex.elems) self(el);},
  [](this auto&& self, LatexElem& elem){
    auto command = std::get_if<LatexCommand>(&elem.kind);
    if (command && command->name == "textbackslash"){
      assert(command->args.size() <= 1);
      if (command->args.size() == 1)
        assert(command->args[0].contents.elems.size() == 0);
      elem = LatexElem{LatexText{"\\"}};
    } else {
      std::visit(self, elem.kind);
    }
  },
  [](this auto&& self, LatexText&){},
  [](this auto&& self, LatexCommand& command){for (auto& arg : command.args) self(arg);},
  [](this auto&& self, LatexArg& arg){self(arg.contents);}
};

const auto drop_space_commands = Overload{
  [](this auto&& self, Latex& latex){for (auto& el : latex.elems) self(el);},
  [](this auto&& self, LatexElem& elem){
    auto command = std::get_if<LatexCommand>(&elem.kind);
    if (command && (command->name == "quad" || command->name == "," ||
                    command->name == "bnfindent")){
      assert(command->args.empty());
      elem = LatexElem{LatexText{""}}; // TODO: try space
    } else {
      std::visit(self, elem.kind);
    }
  },
  [](this auto&& self, LatexText&){},
  [](this auto&& self, LatexCommand& command){for (auto& arg : command.args) self(arg);},
  [](this auto&& self, LatexArg& arg){self(arg.contents);}
};

const auto kill_keyword = Overload{
  [](this auto&& self, Latex& latex){for (auto& el : latex.elems) self(el);},
  [](this auto&& self, LatexElem& elem){
    auto command = std::get_if<LatexCommand>(&elem.kind);
    if (command && command->name == "keyword"){
      assert(command->args.size() == 1);
      assert(command->args[0].contents.elems.size() == 1);
      auto text = std::get_if<LatexText>(&command->args[0].contents.elems[0].kind);
      assert(text != nullptr);
      elem = LatexElem{*text};
    } else {
      std::visit(self, elem.kind);
    }
  },
  [](this auto&& self, LatexText&){},
  [](this auto&& self, LatexCommand& command){for (auto& arg : command.args) self(arg);},
  [](this auto&& self, LatexArg& arg){self(arg.contents);}
};

const auto concat_texts = Overload{
  [](this auto&& self, Latex& latex){
    for (auto& el : latex.elems) self(el);
    // meat and potatoes
    bool prev_text = false;
    size_t store_idx = 0;
    for (size_t idx = 0; idx < latex.elems.size(); ++idx){
      auto text = std::get_if<LatexText>(&latex.elems[idx].kind);
      if (text && text->text.empty()) continue;
      if (text == nullptr || !prev_text){
        prev_text = text != nullptr;
        if (idx != store_idx) latex.elems[store_idx] = std::move(latex.elems[idx]);
        ++store_idx;
        continue;
      }
      auto ptext = std::get_if<LatexText>(&latex.elems[store_idx-1].kind);
      assert(ptext != nullptr);
      ptext->text += text->text;
    }
    latex.elems.erase(latex.elems.begin() + store_idx, latex.elems.end());
  },
  [](this auto&& self, LatexElem& elem){std::visit(self, elem.kind);},
  [](this auto&& self, LatexText&){},
  [](this auto&& self, LatexCommand& command){for (auto& arg : command.args) self(arg);},
  [](this auto&& self, LatexArg& arg){self(arg.contents);}
};

const auto trim_whitespace = Overload{
  [](this auto&& self, Latex& latex){for (auto& el : latex.elems) self(el);},
  [](this auto&& self, LatexElem& elem){std::visit(self, elem.kind);},
  [](this auto&& self, LatexText& text){
    while (!text.text.empty() && ::isspace(text.text.back())) text.text.pop_back();
    while (!text.text.empty() && ::isspace(text.text.front())) text.text.erase(text.text.begin());
  },
  [](this auto&& self, LatexCommand& command){for (auto& arg : command.args) self(arg);},
  [](this auto&& self, LatexArg& arg){self(arg.contents);}
};

const auto count_commands = Overload{
  [](this auto&& self, Latex& latex, auto& out){
    for (auto& el : latex.elems) self(el, out);
  },
  [](this auto&& self, LatexElem& elem, auto& out){
    std::visit([&](auto& x){return self(x, out);}, elem.kind);
  },
  [](this auto&& self, LatexText&, auto&){},
  [](this auto&& self, LatexCommand& command, auto& out){
    ++out[command.name];
    for (auto& arg : command.args) self(arg, out);
  },
  [](this auto&& self, LatexArg& arg, auto& out){self(arg.contents, out);}
};

const auto count_def_commands = Overload{
  [](this auto& self, const Definition& def, auto& counts){
    for (auto&& row : def.disjunction) self(row, counts);
  },
  [](this auto& self, const DefinitionRow& row, auto& counts){
    for (auto&& elem : row.conjunction) self(elem, counts);
  },
  [](this auto& self, const DefinitionElem& elem, auto& counts){
    std::visit([&](auto&& arg){return self(arg, counts);}, elem.kind);
  },
  [](this auto& self, const DefinitionText&, auto&){},
  [](this auto& self, const DefinitionCommand& command, auto& counts){
    ++counts[command.name];
    for (auto&& elem : command.contents) self(elem, counts);
  }
};

int main(){
  // auto keywords_vec = find_keywords();
  // std::set<std::string> keywords(keywords_vec.begin(), keywords_vec.end());
  // find_terminal_keywords();
  // auto keywords2 = find_keywords2();
  // std::cerr << "Only in first:" << std::endl;
  // for (auto& kw : keywords)
  //   if (!keywords2.count(kw))
  //     LOG(kw);
  // std::cerr << "Only in second:" << std::endl;
  // for (auto& kw : keywords2)
  //   if (!keywords.count(kw))
  //     LOG(kw);
  // return 0;
  
  std::filesystem::path file{"/home/ilazaric/repos/draft/source/std-gram.ext"};
  Latex latex = parse_latex_file(file);
  kill_caret(latex);
  kill_escapes(latex);
  kill_textbackslash(latex);
  drop_space_commands(latex);
  concat_texts(latex);
  kill_rlap(latex);
  concat_texts(latex);
  kill_keyword(latex);
  concat_texts(latex);
  trim_whitespace(latex);
  concat_texts(latex); // to remove empty strings
  // std::map<std::string, size_t> command_counts;
  // count_commands(latex, command_counts);
  // for (auto&& [command, count] : command_counts) LOG(command, count);
  // return 0;
  auto envs = find_environments(latex);
  LOG(envs.size());
  std::map<std::string, size_t> counts;
  for (auto&& env : envs){
    Definition def = parse_bnf(env);
    // if (def.term == "operator"){
    // LOG(def.term, def.error);
    assert(!def.error);
    if (def.one_of){
    dump(def);
    // drop_space_commands(def);
    // dump(def);
    normalize_one_of(def);
    dump(def);
    }
    count_def_commands(def, counts);
    // dump(def);
    // }
    // if (def.term == "n-char-sequence") exit(1);
  }
  for (auto [command, count] : counts) LOG(command, count);
  // std::map<std::string, size_t> counts;
  // for (auto&& env : envs) counts[env.name]++;
  // for (auto&& [name, count] : counts) LOG(name, count);
  // dump(latex, 0);
  //
  //
  //
  // TODO: \quad is whitespace
  //       one of is weird
  //       textnormal sucks
}
