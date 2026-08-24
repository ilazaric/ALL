#pragma once

// this file shouldn't be included, it is used by build system

#include <ivl/command_line_argument_parsing/parse>
#include <meta>
#include <print>
#include <string>
#include <string_view>

namespace ivl::main_synthesis {
struct search_result_t {
  std::meta::info main_type = ^^int();
  std::meta::info ivl_main_arg_type = ^^void;
  bool validated = false;
  bool emit_main = false;
  bool passthrough = false;
};

consteval search_result_t find_main_declarations() {
  std::vector<std::meta::info> main_decls;
  std::vector<std::meta::info> ivl_main_decls;
  for (auto member : members_of(^^::, std::meta::access_context::unchecked())) {
    if (!is_function(member)) continue;
    if (!has_identifier(member)) continue;
    auto id = identifier_of(member);
    if (id == "main") main_decls.push_back(member);
    if (id == "ivl_main") ivl_main_decls.push_back(member);
  }

  search_result_t res;

  if (main_decls.size() + ivl_main_decls.size() >= 2) {
    __builtin_constexpr_diag(2, "ivl_main_handler", "too many `main` / `ivl_main` entry points declared");
    return res;
  }

  if (main_decls.empty() && ivl_main_decls.empty()) {
    // Doesn't matter.
    res.main_type = ^^int();
    return res;
  }

  if (!main_decls.empty()) {
    res.main_type = type_of(main_decls[0]);
    return res;
  }

  auto decl = ivl_main_decls[0];
  auto params = parameters_of(decl);
  if (params.size() > 2) {
    __builtin_constexpr_diag(2, "ivl_main_handler", "unexpected number of arguments to `ivl_main` entry point");
    return res;
  }
  res.passthrough = params.size() == 2;
  res.main_type = ^^int(int, char**);
  res.ivl_main_arg_type = params.empty() ? ^^void : decay(type_of(params[0]));
  res.validated = params.empty() || ::ivl::cmdline_parsing::validate_sanity(res.ivl_main_arg_type);
  res.emit_main = true;
  return res;
}

constexpr search_result_t search_result = find_main_declarations();

template<typename arg_t, bool passthrough>
int wrap_ivl_main(int argc, char** argv)
#ifdef __cpp_exceptions
  try
#endif
{
  if constexpr (^^arg_t == ^^void) {
    return [:(passthrough ? ^^:: : ^^::):] ::ivl_main();
  } else {
    arg_t arg{};

    cmdline_parsing::raw_arguments args((const char**)argv + 1, (const char**)argv + argc);

    std::string_view program_name = argc ? argv[0] : "<program-name>";

    if constexpr (
      (!is_class_type(^^arg_t) || reflection::is_child_of(^^arg_t, ^^std)) && !ivl::cmdline_parsing::parseable<arg_t>
    ) {
      static_assert(false);
      return 1;
      // static_assert(^^arg_t != ^^bool, "cannot use bool directly");
      // return ivl_main(construct.template operator()<arg_t>());
    } else {
      // TODO: passthrough
      if (!::ivl::cmdline_parsing::parse(arg, args)) {
      help:
        ::ivl::cmdline_parsing::print_help<arg_t>(program_name, passthrough);
        return 1;
      }
      if constexpr (passthrough) {
        return ivl_main(arg, args);
      } else {
        if (args.empty()) return ivl_main(arg);
        std::println("program does not handle passthrough arguments");
        goto help;
      }
    }
  }
}
#ifdef __cpp_exceptions
catch (const std::exception& e) {
  std::println(stderr, "exception reached main\n{}", e.what());
  return 1;
}
#endif

template<bool use_ivl, typename arg_t, bool passthrough>
int main_template(int argc, char** argv) {
  if constexpr (use_ivl) {
    return wrap_ivl_main<arg_t, passthrough>(argc, argv);
  } else {
    return 0;
  }
}
} // namespace ivl::main_synthesis

namespace {
namespace hide_decl {
  int main(int, char**);
} // namespace hide_decl
} // namespace

[:ivl::main_synthesis::search_result.main_type:] main;

int[:ivl::main_synthesis::search_result.emit_main ? ^^:: : ^^hide_decl:] ::main(int argc, char** argv) {
  return ivl::main_synthesis::main_template < ivl::main_synthesis::search_result.emit_main &&
           ivl::main_synthesis::search_result.validated,
         typename[:ivl::main_synthesis::search_result.emit_main ? ivl::main_synthesis::search_result.ivl_main_arg_type
                                                                : ^^void:], ivl::main_synthesis::search_result
                                                                                .passthrough > (argc, argv);
}
