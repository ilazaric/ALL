#pragma once

// this file shouldn't be included, it is used by build system
// TODO: might want to move it somewhere to reflect ^ then

#include <ivl/command_line_argument_parsing/parsers>
#include <ivl/command_line_argument_parsing/print_help>
#include <meta>
#include <print>
#include <string>
#include <string_view>
#include <tuple>

namespace ivl::main_synthesis {
// TODO: maybe move to ivl/utility
template<typename... Ts>
struct default_initialized {
  template<std::size_t I>
  auto get() const {
    return Ts... [I] {};
  }
};
} // namespace ivl::main_synthesis

template<typename... Ts>
struct std::tuple_size<ivl::main_synthesis::default_initialized<Ts...>>
    : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template<std::size_t I, typename... Ts>
struct std::tuple_element<I, ivl::main_synthesis::default_initialized<Ts...>> {
  using type = Ts...[I];
};

namespace ivl::main_synthesis {
struct search_result_t {
  std::meta::info main_type = ^^int();
  std::meta::info ivl_main_type = ^^int();
  // bool validated = false;
  bool emit_main = false;
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
  res.main_type = ^^int(int, char**);
  res.ivl_main_type = type_of(decl);
  // res.validated = true;
  // for (auto param : params)
  //   if (!::ivl::cmdline_parsing::validate_sanity(decay(type_of(param)))) res.validated = false;
  res.emit_main = true;
  return res;
}

constexpr search_result_t search_result = find_main_declarations();

  template<typename... Args>
int wrap_ivl_main(int argc, char** argv)
#ifdef __cpp_exceptions
  try
#endif
{
  auto [... main_args] = ::ivl::main_synthesis::default_initialized<std::decay_t<Args>...>();
  cmdline_parsing::raw_arguments raw_args((const char**)argv + !!argc, (const char**)argv + argc);
  bool parse_check = ((::ivl::cmdline_parsing::parser<std::decay_t<Args>>{}.parse(main_args, raw_args)) && ...);
  if (parse_check && raw_args.empty()) {
    return [:sizeof...(Args)?^^:::^^:::]::ivl_main(static_cast<Args&&>(main_args)...);
  } else {
    if (parse_check) std::println(stderr, "too many arguments, unparsed: {::?}", raw_args.rest);
    std::string_view program_name = argc ? argv[0] : "<program-name>";
    ::ivl::cmdline_parsing::print_help<std::decay_t<Args>...>(program_name);
    return 1;
  }
}
#ifdef __cpp_exceptions
catch (const std::exception& e) {
  std::println(stderr, "exception reached main\n{}", e.what());
  return 1;
}
#endif

template<bool use_ivl, typename T>
int main_template(int argc, char** argv) {
  if constexpr (use_ivl) {
    return [:substitute((^^wrap_ivl_main), parameters_of(^^T)):](argc, argv);
    // return wrap_ivl_main<T>(argc, argv);
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
  return ivl::main_synthesis::main_template<
    ivl::main_synthesis::search_result.emit_main, typename[:ivl::main_synthesis::search_result.ivl_main_type:]>(
    argc, argv
  );
}
