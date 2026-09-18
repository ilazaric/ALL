#pragma once

#include <ivl/command_line_argument_parsing/parser_one>
#include <ivl/command_line_argument_parsing/parsers/floats>
#include <ivl/format>
#include <ivl/stl/string>
#include <vector>

struct equalizer_config {
  std::vector<double> dbs;
  std::vector<double> freqs;

  double get_db(double freq) const {
    contract_assert(dbs.size() == freqs.size() + 1);
    for (size_t i = 0; i < freqs.size(); ++i)
      if (freq < freqs[i]) return dbs[i];
    return dbs.back();
  }
};

// "+1db < 20hz < -1db < 100hz < +0.5db"
template<>
struct ivl::cmdline_parsing::parser<equalizer_config> : ivl::cmdline_parsing::parser_one {
  bool parse_one(equalizer_config& cfg, std::string_view arg) const {
    auto pieces = ivl::split_py_view(arg);
    for (size_t i = 0; i < pieces.size(); ++i) {
      auto curr = pieces[i];
      auto error = [=]<typename... Ts>(ivl::fmt::format_string<Ts...> fmt, Ts&&... args) {
        ivl::fmt::println(
          "failed to parse equalizer_config\n"
          "- arg: {:?}\n"
          "- current piece: {:?}\n"
          "- {}",
          arg, curr, ivl::fmt::format(fmt, args...)
        );
        return false;
      };
      if (i % 2 == 1) {
        if (curr == "<") continue;
        return error("expected \"<\"");
      }
      if (i % 4 == 0 && !curr.ends_with("db")) return error("expected \"db\" suffix");
      if (i % 4 == 2 && !curr.ends_with("hz")) return error("expected \"hz\" suffix");
      curr.remove_suffix(2);
      parser<double> p;
      double d;
      if (!p.parse_one(d, curr)) return error("failed to parse double");
      (i % 4 == 0 ? cfg.dbs : cfg.freqs).push_back(d);
    }
    return true;
  }
};
