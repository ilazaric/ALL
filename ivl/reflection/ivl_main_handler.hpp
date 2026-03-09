// this file shouldn't be included, it is used by build system

#include <ivl/meta>
#include <ivl/reflection/argument_parsing>
#include <ivl/reflection/prettier_types>
#include <ivl/reflection/utility>
#include <charconv>
#include <filesystem>
#include <meta>
#include <optional>
#include <print>
#include <string>
#include <string_view>

// TODO: add method of enabling parsing of custom class, should be able to consume multiple cmdline entries

/*
  TODO: probably purge this comment, argparsing has been moved

  ivl_main() has to accept a single type ArgT
  decay(ArgT) must satisfy IvlMainArg

  IvlMainArg(T):
  * T must be a non-union class type
  * simple types: bool, ints, floats, enums, std::string{,_view}, std::filesystem::path
  * every member is either a simple type, std::optional<non-bool simple type>, or non-std class
  * no member is a reference, pointer, array, const or volatile qualified
  * for every member whose type is non-std class, type must satisfy IvlMainArg

  argument parsing:
  * `--help` dumps description of all arguments
  * each simple or optional type potentially nested in non-std classes corresponds to an argument
  * identifier list of an argument is the list of identifiers of wrapping non-std classes,
    together with identifier of the terminal object
    * in `struct { struct { int x; } y; };` only argument has ["y", "x"] identifier list
  * name of an argument is concatenation of identifier list with '.' as separator
  * argument is specified on command-line with its name prefixed with "--"
    * in previous example the argument would be specified via "--y.x"
  * if argument is of type bool, it consumes no other element of command-line, sets to true
  * if bool argument is not specified, it is initialized to false
  * if a non-bool simple argument does not have default member initializer, it is mandatory
  * defaulted simple arguments and optional arguments are optional
  * non-bool simple or optional argument consumes exactly one entry of command-line,
    to initialize the variable
 */

// :'(
#ifndef __cpp_exceptions
#define throw [] { asm(""); }(),
#define catch(...) if not consteval
#define try
#define exception(...) exception("", {})
#endif

namespace ivl::main_synthesis {
struct search_result_t {
  std::meta::info main_type;
  std::meta::info ivl_main_arg_type;
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

  if (main_decls.size() + ivl_main_decls.size() >= 2)
    throw std::meta::exception("too many `main` / `ivl_main` entry points declared", {});

  search_result_t res;

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
  if (params.size() == 0 || params.size() > 2)
    throw std::meta::exception("unexpected number of arguments to `ivl_main` entry point", decl);
  res.passthrough = params.size() == 2;
  res.main_type = ^^int(int, char**);
  res.ivl_main_arg_type = decay(type_of(params[0]));
  res.validated = ::ivl::cmdline_parsing::validate_sanity(res.ivl_main_arg_type);
  res.emit_main = true;
  return res;
}

constexpr search_result_t search_result = find_main_declarations();

template<typename arg_t, bool passthrough>
int wrap_ivl_main(int argc, char** argv) {
  try {
    arg_t arg{};

    std::span<const char*> args((const char**)argv + 1, (const char**)argv + argc);

    std::string_view program_name = argc ? argv[0] : "<program-name>";

    if constexpr (!is_class_type(^^arg_t) || reflection::is_child_of(^^arg_t, ^^std)) {
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
  } catch (const std::exception& e) {
#ifdef __cpp_exceptions
    std::println(stderr, "exeption reached main\n{}", e.what());
    return 1;
#endif
  }
}

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
