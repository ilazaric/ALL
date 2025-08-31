#pragma once

#include <ivl/fs/fileview.hpp>
#include <ivl/logger>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// https://texdoc.org/serve/latex2e.pdf/0

namespace ivl::cppp {

  struct LatexText {
    std::string text;
    bool        operator==(const LatexText&) const = default;
  };

  struct LatexArg;

  struct LatexCommand {
    std::string           name;
    std::vector<LatexArg> args;
    bool                  operator==(const LatexCommand&) const = default;
  };

  struct LatexElem {
    std::variant<LatexText, LatexCommand> kind;
    bool                                  operator==(const LatexElem&) const = default;
  };

  struct Latex {
    std::vector<LatexElem> elems;

    void merge(Latex&& other) {
      for (auto& el : other.elems)
        elems.emplace_back(std::move(el));
    }

    bool operator==(const Latex&) const = default;
  };

  struct LatexArg {
    enum Kind { CURLY, BRACKET } kind;
    Latex contents;
    bool  operator==(const LatexArg&) const = default;
  };

  size_t seek_while(std::string_view data, auto&& pred) {
    size_t idx = 0;
    while (idx < data.size() && pred(data[idx]))
      ++idx;
    return idx;
  }

  void skip_whitespace(std::string_view& data) { data.remove_prefix(seek_while(data, (int (&)(int))&std::isspace)); }

  std::string consume_command_name(std::string_view& data) {
    assert(!data.empty());
    size_t      idx = std::isalpha(data[0]) ? seek_while(data, (int (&)(int))&std::isalpha) : 1;
    std::string ret(data.substr(0, idx));
    data.remove_prefix(idx);
    return ret;
  }

  void skip_comment(std::string_view& data) {
    if (data.empty() || data[0] != '%') return;
    data = data.substr(data.find('\n'));
    if (!data.empty()) data.remove_prefix(1);
  }

#define DBG(...)                                                                                                       \
  do {                                                                                                                 \
  } while (0)
  // #define DBG(...) do { __VA_ARGS__ } while (0)

  std::string_view original_data;
  size_t           dbg_indent = -1;
  // #define INDENT dbg_indent << "\t"
#define INDENT std::string(dbg_indent * 2, ' ')
  Latex parse_latex_string(std::string_view& data, const char terminator) {
    dbg_indent += 1;
    struct reverter {
      ~reverter() { dbg_indent -= 1; }
    } reverter;

    DBG(std::cerr << INDENT << "parsing..."; if (terminator) std::cerr << terminator; std::cerr << std::endl;);

    if (terminator == '\0') original_data = data;

#define my_assert(...)                                                                                                 \
  do {                                                                                                                 \
    if (!(__VA_ARGS__)) {                                                                                              \
      std::string_view prefix       = original_data.substr(0, original_data.size() - data.size());                     \
      size_t           newlinecount = 0;                                                                               \
      while (true) {                                                                                                   \
        size_t p = prefix.find('\n');                                                                                  \
        if (p == std::string_view::npos) break;                                                                        \
        ++newlinecount;                                                                                                \
        prefix.remove_prefix(p + 1);                                                                                   \
      }                                                                                                                \
      std::cerr << "Location: " << newlinecount + 1 << ":" << prefix.size() << std::endl;                              \
      std::cerr << "Text:\n" << data.substr(0, 200) << std::endl << std::endl;                                         \
      assert(__VA_ARGS__);                                                                                             \
    }                                                                                                                  \
  } while (0)

    Latex res;
    while (!data.empty()) {
      if (::isspace(data[0])) {
        res.elems.emplace_back(LatexText{" "});
      }

      skip_whitespace(data);
      skip_comment(data);

      // end of arg
      if (data[0] == terminator) {
        break;
      }

      // command
      if (data[0] == '\\') {
        data.remove_prefix(1);
        res.elems.emplace_back(LatexCommand{});
        LatexCommand& command = std::get<LatexCommand>(res.elems.back().kind);
        command.name          = consume_command_name(data);
        DBG(std::cerr << INDENT << "Command: " << command.name << std::endl;);
        // sanity check
        if (!data.empty()) {
          if (data[0] == terminator) break;
        }

        while (true) {
          if (data.empty()) break;
          if (data[0] != '[' && data[0] != '{') break;
          char open = data[0];
          if (command.name == "keyword" && open == '[') break;
          char close = open == '[' ? ']' : '}';
          data.remove_prefix(1);
          command.args.emplace_back(
            open == '[' ? LatexArg::Kind::BRACKET : LatexArg::Kind::CURLY, parse_latex_string(data, close)
          );
          my_assert(!data.empty());
          my_assert(data[0] == close);
          data.remove_prefix(1);
        }
        continue;
      }

      // text
      res.elems.emplace_back(LatexText{});
      LatexText& text = std::get<LatexText>(res.elems.back().kind);

      while (!data.empty() && data[0] != '\\' && data[0] != terminator) {
        skip_comment(data);
        if (!(!data.empty() && data[0] != '\\' && data[0] != terminator)) break;
        text.text += data[0];
        data.remove_prefix(1);
      }
      DBG(std::cerr << INDENT << "Text: "; std::cerr << text.text << std::endl;);
    }
    return res;
  }

  Latex parse_latex_file(const std::filesystem::path& file) {
    fs::FileView     fv(file.native());
    std::string_view sv = fv.as_string_view();
    assert(sv.find('\0') == std::string_view::npos);
    Latex ret = parse_latex_string(sv, '\0');
    assert(sv.empty());
    return ret;
  }

  Latex parse_latex_files(const std::vector<std::filesystem::path>& files) {
    Latex latex;
    for (const auto& file : files)
      latex.merge(parse_latex_file(file));
    return latex;
  }

  struct LatexEnvironment {
    std::string  name;
    LatexCommand begin_command;
    LatexCommand end_command;
    Latex        contents;
  };

  std::string_view environment_edge_name(const LatexElem& elem) {
    const LatexCommand& command = std::get<LatexCommand>(elem.kind);
    assert(!command.args.empty());
    const LatexArg& arg = command.args[0];
    assert(arg.kind == LatexArg::Kind::CURLY);
    const Latex& arg_latex = arg.contents;
    assert(arg_latex.elems.size() == 1);
    return std::get<LatexText>(arg_latex.elems[0].kind).text;
  }

  std::vector<LatexEnvironment> find_environments(const Latex& latex) {
    std::vector<size_t>           begin_stack;
    std::vector<LatexEnvironment> ret;
    for (size_t idx = 0; idx < latex.elems.size(); ++idx) {
      auto command = std::get_if<LatexCommand>(&latex.elems[idx].kind);
      if (command == nullptr) continue;
      if (command->name == "begin") {
        begin_stack.push_back(idx);
        continue;
      }
      if (command->name == "end") {
        assert(!begin_stack.empty());
        assert(environment_edge_name(latex.elems[begin_stack.back()]) == environment_edge_name(latex.elems[idx]));
        ret.emplace_back(
          std::string(environment_edge_name(latex.elems[idx])),
          std::get<LatexCommand>(latex.elems[begin_stack.back()].kind), std::get<LatexCommand>(latex.elems[idx].kind),
          Latex{}
        );
        for (size_t bla = begin_stack.back() + 1; bla < idx; ++bla)
          ret.back().contents.elems.push_back(latex.elems[bla]);
        begin_stack.pop_back();
        continue;
      }
    }
    assert(begin_stack.empty());
    return ret;
  }

} // namespace ivl::cppp
