#include "parse_latex.hpp"

#include <boost/algorithm/string/split.hpp>
#include <ivl/io/stlutils.hpp>
#include <map>
#include <ranges>
#include <set>

using namespace ivl;
using namespace cppp;

template <typename... Ts>
struct Overload : Ts... {
  using Ts::operator()...;
};
template <typename... Ts>
Overload(Ts...) -> Overload<Ts...>;

template <typename A, typename B>
bool eq(const A& a, const B& b) {
  if constexpr (std::is_same_v<A, LatexElem>) {
    return std::visit([&](auto&& x) { return eq(x, b); }, a.kind);
  } else if constexpr (std::is_same_v<B, LatexElem>) {
    return std::visit([&](auto&& x) { return eq(x, a); }, b.kind);
  } else if constexpr (!std::is_same_v<A, B>) {
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
  std::string                 name;
  std::vector<DefinitionElem> contents;
};

struct DefinitionElem {
  std::variant<DefinitionText, DefinitionCommand> kind;
};

struct DefinitionRow {
  std::vector<DefinitionElem> conjunction;
};

struct Definition {
  std::string                term;
  bool                       error  = false;
  bool                       one_of = false;
  std::vector<DefinitionRow> disjunction;
};

const auto dump_latex = []<typename T>(this const auto& self, const T& entity) {
  if constexpr (std::is_same_v<T, LatexText>) {
    std::cerr << '"' << entity.text << '"';
  } else if constexpr (std::is_same_v<T, LatexCommand>) {
    std::cerr << "\\" << entity.name;
    for (auto& arg : entity.args)
      self(arg);
  } else if constexpr (std::is_same_v<T, LatexArg>) {
    std::cerr << (entity.kind == LatexArg::Kind::CURLY ? '{' : '[');
    self(entity.contents);
    std::cerr << (entity.kind == LatexArg::Kind::CURLY ? '}' : ']');
  } else if constexpr (std::is_same_v<T, Latex>) {
    bool first = true;
    for (auto& el : entity.elems) {
      if (!first)
        std::cerr << " ";
      self(el);
      first = false;
    }
  } else if constexpr (std::is_same_v<T, LatexElem>) {
    std::visit(self, entity.kind);
  } else if constexpr (std::is_same_v<T, LatexEnvironment>) {
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

std::vector<std::string> split(std::string_view sv) {
  std::vector<std::string> res;
  boost::algorithm::split(res, sv, ::isspace, boost::token_compress_on);
  size_t b = 0;
  for (size_t a = 0; a != res.size(); ++a)
    if (!res[a].empty()) {
      if (a != b)
        res[b] = std::move(res[a]);
      ++b;
    }
  res.erase(res.begin() + b, res.end());
  return res;
}

void fillup(std::vector<DefinitionElem>& res, const LatexElem& elem) {
  std::visit(
    [&]<typename T>(const T& x) {
      if constexpr (std::is_same_v<T, LatexText>) {
        auto pieces = split(x.text);
        for (auto&& piece : pieces)
          res.emplace_back(DefinitionText {std::move(piece)});
      } else { // LatexCommand
        res.emplace_back(DefinitionCommand {x.name});
        auto& dc = std::get<DefinitionCommand>(res.back().kind);
        if (x.name != "unicode")
          assert(x.args.size() <= 1);
        else
          assert(x.args.size() == 2);
        for (auto&& arg : x.args | std::views::take(1)) {
          assert(arg.kind == LatexArg::Kind::CURLY);
          for (auto&& el : arg.contents.elems)
            fillup(dc.contents, el);
        }
      }
    },
    elem.kind);
}

Definition parse_bnf(const LatexEnvironment& env) {
  const auto& contents = env.contents.elems;
  size_t      idx      = 0;

  auto skip_whitespace_text = [&] {
    while (idx != contents.size()) {
      auto text = std::get_if<LatexText>(&contents[idx].kind);
      if (text == nullptr)
        return;
      if (std::ranges::all_of(text->text, ::isspace)) {
        ++idx;
        continue;
      } else
        return;
    }
  };

  skip_whitespace_text();
  if (idx != contents.size()) {
    auto comm = std::get_if<LatexCommand>(&contents[idx].kind);
    if (comm != nullptr && comm->name == "microtypesetup")
      ++idx;
  }
  skip_whitespace_text();
  if (idx != contents.size() && eq(contents[idx], LatexCommand {"obeyspaces"}))
    ++idx;

  const auto term = ({
    skip_whitespace_text();
    if (idx == contents.size())
      return {"???", true};
    const auto nontermdef = std::get_if<LatexCommand>(&contents[idx].kind);
    if (nontermdef == nullptr)
      return {"???", true};
    if (nontermdef->name != "nontermdef")
      return {"???", true};
    if (nontermdef->args.size() != 1)
      return {"???", true};
    if (nontermdef->args[0].kind != LatexArg::Kind::CURLY)
      return {"???", true};
    if (nontermdef->args[0].contents.elems.size() != 1)
      return {"???", true};
    const auto nontermdef_term =
      std::get_if<LatexText>(&nontermdef->args[0].contents.elems[0].kind);
    if (nontermdef_term == nullptr)
      return {"???", true};
    ++idx;
    nontermdef_term->text;
  });

  skip_whitespace_text();
  bool one_of = false;
  if (idx != contents.size() &&
      eq(contents[idx], LatexCommand {"textnormal",
                                      {LatexArg {LatexArg::Kind::CURLY,
                                                 Latex {{LatexElem {LatexText {"one of"}}}}}}})) {
    one_of = true;
    ++idx;
  }

  std::vector<DefinitionRow> rows;
  while (true) {
    skip_whitespace_text();
    if (idx == contents.size())
      break;
    if (!eq(contents[idx], LatexCommand {"br"}))
      return {term, true};
    ++idx;
    rows.emplace_back();
    auto& row = rows.back();
    while (true) {
      skip_whitespace_text();
      if (idx == contents.size() || eq(contents[idx], LatexCommand {"br"}))
        break;
      fillup(row.conjunction, contents[idx]);
      ++idx;
    }
  }
  return {term, false, one_of, rows};
}

void dump(const DefinitionElem& elem) {
  visit(
    []<typename T>(const T& x) {
      if constexpr (std::is_same_v<T, DefinitionText>) {
        std::cout << "'" << x.text << "'";
      } else {
        std::cout << x.name << "{";
        for (auto&& e : x.contents) {
          std::cout << " ";
          dump(e);
        }
        std::cout << " }";
      }
    },
    elem.kind);
}

void dump(const DefinitionRow& row) {
  for (auto&& el : row.conjunction) {
    std::cout << " ";
    dump(el);
  }
  std::cout << std::endl;
}

void dump(const Definition& def) {
  std::cout << def.term << " ::=" << std::endl;
  if (def.error) {
    std::cout << "  *** ERROR ***" << std::endl;
    return;
  }
  if (def.one_of) {
    std::cout << "  -- ONE OF --" << std::endl;
  }
  for (auto&& row : def.disjunction) {
    std::cout << "  |";
    dump(row);
  }
}

size_t count_terminals(const Latex& latex, const std::string& text);

size_t count_terminals(const LatexElem& elem, const std::string& kw) {
  auto command = std::get_if<LatexCommand>(&elem.kind);
  if (command == nullptr)
    return 0;
  size_t count = 0;
  for (auto& arg : command->args)
    count += count_terminals(arg.contents, kw);
  if (command->name != "terminal"
      // && command->name != "tcode"
  )
    return count;
  if (command->args.size() != 1) {
    LOG(command->args.size());
    dump_latex(*command);
    std::cerr << std::endl;
  }
  assert(command->args.size() == 1);
  for (auto& el : command->args[0].contents.elems) {
    auto text = std::get_if<LatexText>(&el.kind);
    if (text == nullptr)
      continue;
    auto words = split(text->text);
    for (auto& word : words)
      if (word == kw)
        ++count;
  }
  return count;
}

size_t count_terminals(const Latex& latex, const std::string& text) {
  size_t count = 0;
  for (auto& el : latex.elems)
    count += count_terminals(el, text);
  return count;
}

void normalize_one_of(Definition& def) {
  if (!def.one_of)
    return;
  if (def.error) {
    dump(def);
    assert(false);
  }

  // if (def.term == "operator-or-punctuator"){
  //   dump(def);
  //   std::cout << std::endl;
  // }

  std::vector<DefinitionRow> new_rows;
  for (auto& dis : def.disjunction) {
    for (auto& con : dis.conjunction) {
      auto& comm = std::get<DefinitionCommand>(con.kind);
      assert(comm.name == "terminal");
      for (auto& el : comm.contents) {
        if (auto text = std::get_if<DefinitionText>(&el.kind)) {
          new_rows.emplace_back(DefinitionRow {
            {DefinitionElem {DefinitionCommand {"terminal", {DefinitionElem {*text}}}}}});
          continue;
        }

        // if (def.term == "operator-or-punctuator"){ dump(el); std::cout << std::endl; }

        auto& comm2 = std::get<DefinitionCommand>(el.kind);
        assert(comm2.name == "keyword");
        assert(comm2.contents.size() == 1);
        auto& text = std::get<DefinitionText>(comm2.contents[0].kind);
        new_rows.emplace_back(DefinitionRow {
          {DefinitionElem {DefinitionCommand {"keyword", {DefinitionElem {text}}}}}});

        // if (def.term == "operator-or-punctuator"){ dump(el); std::cout << std::endl; }
      }
    }
  }

  def.one_of      = false;
  def.disjunction = new_rows;
}

const auto kill_caret = []<typename T>(this const auto& self, T& entity) {
  if constexpr (std::is_same_v<T, Latex>) {
    for (auto& el : entity.elems)
      self(el);
  } else if constexpr (std::is_same_v<T, LatexElem>) {
    auto comm = std::get_if<LatexCommand>(&entity.kind);
    if (comm && comm->name == "caret") {
      assert(comm->args.size() <= 1);
      if (comm->args.size() == 1)
        assert(comm->args[0].contents.elems.empty());
      entity = LatexElem {LatexText {"^"}};
    } else {
      std::visit(self, entity.kind);
    }
  } else if constexpr (std::is_same_v<T, LatexText>) {
    // nothing
  } else if constexpr (std::is_same_v<T, LatexCommand>) {
    for (auto& arg : entity.args)
      self(arg.contents);
  } else {
    static_assert(false);
  }
};

std::set<char> escapes {'%', '&', '~', '{', '}', '#'};
const auto     kill_escapes =
  Overload {[](this auto&& self, Latex& latex) {
              for (auto& el : latex.elems)
                self(el);
            },
            [](this auto&& self, LatexElem& elem) {
              auto command = std::get_if<LatexCommand>(&elem.kind);
              if (command && command->name.size() == 1 && escapes.contains(command->name[0])) {
                assert(command->args.empty());
                elem = LatexElem {LatexText {command->name}};
              } else {
                std::visit(self, elem.kind);
              }
            },
            [](this auto&& self, LatexText&) {},
            [](this auto&& self, LatexCommand& command) {
              for (auto& arg : command.args)
                self(arg);
            },
            [](this auto&& self, LatexArg& arg) { self(arg.contents); }};

const auto kill_rlap = Overload {[](this auto&& self, Latex& latex) {
                                   for (auto& el : latex.elems)
                                     self(el);
                                 },
                                 [](this auto&& self, LatexElem& elem) -> void {
                                   auto command = std::get_if<LatexCommand>(&elem.kind);
                                   if (command && command->name == "rlap") {
                                     assert(command->args.size() == 1);
                                     assert(command->args[0].contents.elems.size() == 1);
                                     elem = command->args[0].contents.elems[0];
                                     self(elem);
                                   } else {
                                     std::visit(self, elem.kind);
                                   }
                                 },
                                 [](this auto&& self, LatexText&) {},
                                 [](this auto&& self, LatexCommand& command) {
                                   for (auto& arg : command.args)
                                     self(arg);
                                 },
                                 [](this auto&& self, LatexArg& arg) { self(arg.contents); }};

const auto kill_textbackslash =
  Overload {[](this auto&& self, Latex& latex) {
              for (auto& el : latex.elems)
                self(el);
            },
            [](this auto&& self, LatexElem& elem) {
              auto command = std::get_if<LatexCommand>(&elem.kind);
              if (command && command->name == "textbackslash") {
                assert(command->args.size() <= 1);
                if (command->args.size() == 1)
                  assert(command->args[0].contents.elems.size() == 0);
                elem = LatexElem {LatexText {"\\"}};
              } else {
                std::visit(self, elem.kind);
              }
            },
            [](this auto&& self, LatexText&) {},
            [](this auto&& self, LatexCommand& command) {
              for (auto& arg : command.args)
                self(arg);
            },
            [](this auto&& self, LatexArg& arg) { self(arg.contents); }};

const auto drop_space_commands =
  Overload {[](this auto&& self, Latex& latex) {
              for (auto& el : latex.elems)
                self(el);
            },
            [](this auto&& self, LatexElem& elem) {
              auto command = std::get_if<LatexCommand>(&elem.kind);
              if (command && (command->name == "quad" || command->name == "," ||
                              command->name == "bnfindent")) {
                assert(command->args.empty());
                elem = LatexElem {LatexText {""}}; // TODO: try space
              } else {
                std::visit(self, elem.kind);
              }
            },
            [](this auto&& self, LatexText&) {},
            [](this auto&& self, LatexCommand& command) {
              for (auto& arg : command.args)
                self(arg);
            },
            [](this auto&& self, LatexArg& arg) { self(arg.contents); }};

const auto kill_keyword =
  Overload {[](this auto&& self, Latex& latex) {
              for (auto& el : latex.elems)
                self(el);
            },
            [](this auto&& self, LatexElem& elem) {
              auto command = std::get_if<LatexCommand>(&elem.kind);
              if (command && command->name == "keyword") {
                assert(command->args.size() == 1);
                assert(command->args[0].contents.elems.size() == 1);
                auto text = std::get_if<LatexText>(&command->args[0].contents.elems[0].kind);
                assert(text != nullptr);
                elem = LatexElem {*text};
              } else {
                std::visit(self, elem.kind);
              }
            },
            [](this auto&& self, LatexText&) {},
            [](this auto&& self, LatexCommand& command) {
              for (auto& arg : command.args)
                self(arg);
            },
            [](this auto&& self, LatexArg& arg) { self(arg.contents); }};

const auto concat_texts =
  Overload {[](this auto&& self, Latex& latex) {
              for (auto& el : latex.elems)
                self(el);
              // meat and potatoes
              bool   prev_text = false;
              size_t store_idx = 0;
              for (size_t idx = 0; idx < latex.elems.size(); ++idx) {
                auto text = std::get_if<LatexText>(&latex.elems[idx].kind);
                if (text && text->text.empty())
                  continue;
                if (text == nullptr || !prev_text) {
                  prev_text = text != nullptr;
                  if (idx != store_idx)
                    latex.elems[store_idx] = std::move(latex.elems[idx]);
                  ++store_idx;
                  continue;
                }
                auto ptext = std::get_if<LatexText>(&latex.elems[store_idx - 1].kind);
                assert(ptext != nullptr);
                ptext->text += text->text;
              }
              latex.elems.erase(latex.elems.begin() + store_idx, latex.elems.end());
            },
            [](this auto&& self, LatexElem& elem) { std::visit(self, elem.kind); },
            [](this auto&& self, LatexText&) {},
            [](this auto&& self, LatexCommand& command) {
              for (auto& arg : command.args)
                self(arg);
            },
            [](this auto&& self, LatexArg& arg) { self(arg.contents); }};

const auto trim_whitespace =
  Overload {[](this auto&& self, Latex& latex) {
              for (auto& el : latex.elems)
                self(el);
            },
            [](this auto&& self, LatexElem& elem) { std::visit(self, elem.kind); },
            [](this auto&& self, LatexText& text) {
              while (!text.text.empty() && ::isspace(text.text.back()))
                text.text.pop_back();
              while (!text.text.empty() && ::isspace(text.text.front()))
                text.text.erase(text.text.begin());
            },
            [](this auto&& self, LatexCommand& command) {
              for (auto& arg : command.args)
                self(arg);
            },
            [](this auto&& self, LatexArg& arg) { self(arg.contents); }};

// const auto split_terminal = Overload {
//   [](this auto&& self, Latex& latex) {
//     for (auto& el : latex.elems)
//       self(el);
//     // meat and potatoes
//     for (size_t idx = 0; idx < latex.elems.size(); ++idx) {
//       auto comm = std::get_if<LatexCommand>(&latex.elems[idx].kind);
//       if (!comm || comm->name != "terminal")
//         continue;
//       assert(comm->args.size() == 1);
//       assert(comm->args[0].kind == LatexArg::Kind::CURLY);
//       auto& arg_elems = comm->args[0].contents.elems;
//       if (arg_elems.size() == 1){
//         dump_latex(*comm); std::cerr << std::endl;
//         continue;
//       }
//       assert(arg_elems.size() != 0);
//       std::vector<LatexElem> new_stuff;
//       for (size_t arg_idx = 1; arg_idx < arg_elems.size(); ++arg_idx) {
//         LatexCommand new_comm {"terminal", {}};
//         new_comm.args.emplace_back(LatexArg {LatexArg::CURLY, Latex {}});
//         new_comm.args.back().contents.elems.emplace_back(std::move(arg_elems[arg_idx]));
//         new_stuff.emplace_back(std::move(new_comm));
//       }
//       arg_elems.resize(1);
//       latex.elems.insert(latex.elems.begin() + idx + 1, new_stuff.begin(), new_stuff.end());
//     }
//   },
//   [](this auto&& self, LatexElem& elem) { std::visit(self, elem.kind); },
//   [](this auto&& self, LatexText&) {},
//   [](this auto&& self, LatexCommand& command) {
//     for (auto& arg : command.args)
//       self(arg);
//   },
//   [](this auto&& self, LatexArg& arg) { self(arg.contents); }};

const auto count_commands =
  Overload {[](this auto&& self, Latex& latex, auto& out) {
              for (auto& el : latex.elems)
                self(el, out);
            },
            [](this auto&& self, LatexElem& elem, auto& out) {
              std::visit([&](auto& x) { return self(x, out); }, elem.kind);
            },
            [](this auto&& self, LatexText&, auto&) {},
            [](this auto&& self, LatexCommand& command, auto& out) {
              ++out[command.name];
              for (auto& arg : command.args)
                self(arg, out);
            },
            [](this auto&& self, LatexArg& arg, auto& out) { self(arg.contents, out); }};

const auto count_def_commands =
  Overload {[](this auto& self, const Definition& def, auto& counts) {
              for (auto&& row : def.disjunction)
                self(row, counts);
            },
            [](this auto& self, const DefinitionRow& row, auto& counts) {
              for (auto&& elem : row.conjunction)
                self(elem, counts);
            },
            [](this auto& self, const DefinitionElem& elem, auto& counts) {
              std::visit([&](auto&& arg) { return self(arg, counts); }, elem.kind);
            },
            [](this auto& self, const DefinitionText&, auto&) {},
            [](this auto& self, const DefinitionCommand& command, auto& counts) {
              ++counts[command.name];
              for (auto&& elem : command.contents)
                self(elem, counts);
            }};

constexpr auto fix_name = [](std::string name) {
  for (auto& c : name)
    if (c == '-')
      c = '_';
  return "entity_" + name;
};

constexpr auto escape_doublequote = [](std::string s) { return "R\"raw(" + s + ")raw\""; };

void dump_as_cpp(const Definition& def) {
  assert(!def.error);
  assert(!def.one_of);
  assert(!def.disjunction.empty());
  for (auto&& d : def.disjunction)
    assert(!d.conjunction.empty());

  // if (def.term == "operator-or-punctuator"){
  //   dump(def);
  //   std::cout << std::endl;
  // }

  {
    if (def.disjunction.size() >= 3)
      goto list_end;
    if (def.disjunction.size() == 1) {
      auto&& dis = def.disjunction[0];
      // A = B opt(A)
      if (dis.conjunction.size() != 2)
        goto list_end;
      auto bla  = std::get_if<DefinitionText>(&dis.conjunction[0].kind);
      auto truc = std::get_if<DefinitionCommand>(&dis.conjunction[1].kind);
      if (!bla || !truc || truc->name != "opt" || truc->contents.size() != 1)
        goto list_end;
      auto znj = std::get_if<DefinitionText>(&truc->contents[0].kind);
      if (!znj || znj->text != def.term)
        goto list_end;
      std::cout << "ENTITY(" << fix_name(def.term) << ", List<Entity<" << fix_name(bla->text)
                << ">>);\n";
      return;
    }
    // A = B | B A
    auto&& d1 = def.disjunction[0].conjunction.size() == 1 ? def.disjunction[0].conjunction
                                                           : def.disjunction[1].conjunction;
    auto&& d2 = def.disjunction[0].conjunction.size() == 1 ? def.disjunction[1].conjunction
                                                           : def.disjunction[0].conjunction;
    if (d1.size() != 1 || d2.size() != 2)
      goto list_end;
    auto bla   = std::get_if<DefinitionText>(&d1[0].kind);
    auto truc1 = std::get_if<DefinitionText>(&d2[0].kind);
    auto truc2 = std::get_if<DefinitionText>(&d2[1].kind);
    if (!bla || !truc1 || !truc2)
      goto list_end;
    if (bla->text == truc1->text && def.term == truc2->text ||
        bla->text == truc2->text && def.term == truc1->text) {
      std::cout << "ENTITY(" << fix_name(def.term) << ", List<Entity<" << fix_name(bla->text)
                << ">>);\n";
      return;
    }
  }
list_end:;

  {
    bool has_self_start = false;
    for (auto&& dis : def.disjunction) {
      assert(dis.conjunction.size() >= 1);
      auto&& first = dis.conjunction[0];
      auto   text  = std::get_if<DefinitionText>(&first.kind);
      if (!text)
        continue;
      if (text->text == def.term)
        has_self_start = true;
    }
    if (!has_self_start)
      goto self_start_end;
    Definition def_start;
    Definition def_continue;
    def_start.term    = def.term + "-impl-start";
    def_continue.term = def.term + "-impl-continue";
    std::cout << "ENTITY(" << fix_name(def.term) << ", And<Entity<" << fix_name(def_start.term)
              << ">, List<Entity<" << fix_name(def_continue.term) << ">, 0>>);" << std::endl;
    for (auto&& dis : def.disjunction) {
      auto text = std::get_if<DefinitionText>(&dis.conjunction[0].kind);
      if (text && text->text == def.term) {
        def_continue.disjunction.emplace_back(dis);
        def_continue.disjunction.back().conjunction.erase(
          def_continue.disjunction.back().conjunction.begin());
      } else {
        def_start.disjunction.emplace_back(dis);
      }
    }
    dump_as_cpp(def_start);
    dump_as_cpp(def_continue);
    return;
  }
self_start_end:;

  std::cout << "ENTITY(" << fix_name(def.term) << ", Or<";
  bool dis_first = true;
  for (auto&& dis : def.disjunction) {
    if (!dis_first)
      std::cout << ", ";
    dis_first = false;
    std::cout << "And<";
    bool con_first = true;
    for (auto&& con : dis.conjunction) {
      if (!con_first)
        std::cout << ", ";
      con_first         = false;
      const auto dumper = Overload {
        [](this auto&& self, const DefinitionElem& elem) { std::visit(self, elem.kind); },
        [](this auto&& self, const DefinitionText& text) {
          std::cout << "Entity<" << fix_name(text.text) << ">";
        },
        [](this auto&& self, const DefinitionCommand& comm) {
          if (comm.name == "opt") {
            assert(comm.contents.size() == 1);
            std::cout << "Opt<";
            self(comm.contents[0]);
            std::cout << ">";
            return;
          }
          if (comm.name == "terminal" || comm.name == "keyword") {
            auto name = comm.name;
            name[0]   = toupper(name[0]);
            bool znj  = true;
            for (auto&& content : comm.contents) {
              if (!znj)
                std::cout << ", ";
              znj    = false;
              auto x = std::get_if<DefinitionText>(&content.kind);
              if (!x) {
                std::cout << "UnimplementedTODO<> /* ";
                dump(content);
                std::cout << " */";
              } else {
                std::cout << name << "<" << escape_doublequote(x->text) << ">";
              }
            }
            return;
          }
        dumper_fail: {
          std::cout << "UnimplementedTODO<> /* ";
          dump(DefinitionElem {comm});
          std::cout << " */";
        }
        },
      };
      dumper(con);
    }
    std::cout << ">";
  }
  std::cout << ">);\n";

  // std::cout << "ENTITY(" << fix_name(def.term) << ", UnimplementedTODO);\n";
}

int main() {
  std::filesystem::path file {"/home/ilazaric/repos/draft/source/std-gram.ext"};
  // std::filesystem::path file {"/home/ilazaric/repos/draft/source/small.ext"};
  Latex latex = parse_latex_file(file);
  kill_caret(latex);
  kill_escapes(latex);
  kill_textbackslash(latex);
  drop_space_commands(latex);
  // trim_whitespace(latex);
  concat_texts(latex);
  kill_rlap(latex);
  concat_texts(latex);
  // kill_keyword(latex);
  // concat_texts(latex);
  trim_whitespace(latex);
  concat_texts(latex); // to remove empty strings
  // split_terminal(latex);
  auto envs = find_environments(latex);
  LOG(envs.size());
  std::map<std::string, size_t> counts;
  for (auto&& env : envs) {
    Definition def = parse_bnf(env);
    LOG(def.term);
    if (def.term == "new-line")
      continue;
    // if (def.term == "identifier") continue;
    // if (def.term == "identifier-start") continue;
    // if (def.term == "identifier-continue") continue;
    assert(!def.error);
    normalize_one_of(def);
    count_def_commands(def, counts);
    // dump(def);
    dump_as_cpp(def);
  }
  for (auto [command, count] : counts)
    LOG(command, count);

  // TODO: \quad is whitespace
  //       one of is weird
  //       textnormal sucks
}
