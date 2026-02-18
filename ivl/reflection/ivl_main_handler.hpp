// this file shouldn't be included, it is used by build system

#include <ivl/meta>
#include <ivl/reflection/prettier_types>
#include <ivl/reflection/util>
#include <charconv>
#include <filesystem>
#include <meta>
#include <optional>
#include <print>
#include <string>
#include <string_view>

// TODO: add method of enabling parsing of custom class, should be able to consume multiple cmdline entries

/*
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
// TODO: allow `const char*` ?
consteval void validate_arg(const std::meta::info arg) {
  try {
    auto ti = dealias(arg);

    auto check_plain = [](std::meta::info ti) {
      if (is_reference_type(ti)) throw std::meta::exception("cannot parse reference types", {});
      if (is_pointer_type(ti)) throw std::meta::exception("cannot parse pointer types", {});
      if (is_array_type(ti)) throw std::meta::exception("cannot parse array types", {});
      if (is_const(ti)) throw std::meta::exception("cannot parse const types", {});
      if (is_volatile(ti)) throw std::meta::exception("cannot parse volatile types", {});
      if (is_same_type(ti, ^^std::optional<bool>)) throw std::meta::exception("bool cannot be optional-wrapped", {});
    };

    check_plain(ti);

    auto is_simple = [](std::meta::info ti) {
      return is_floating_point_type(ti) || is_integral_type(ti) || is_enum_type(ti) ||
             is_same_type(ti, ^^std::string) || is_same_type(ti, ^^std::string_view) ||
             is_same_type(ti, ^^std::filesystem::path);
    };

    if (is_simple(ti)) return;

    if (ti == ^^std::optional<bool>) throw std::meta::exception("bool cannot be optional-wrapped", {});

    if (has_template_arguments(ti) && template_of(ti) == ^^std::optional) {
      auto opti = template_arguments_of(ti)[0];
      check_plain(opti);
      if (is_simple(opti)) return;
    }

    if (!is_class_type(ti) || ivl::reflection::is_child_of(ti, ^^std))
      throw std::meta::exception("cannot handle this type", {});

    for (auto member : nonstatic_data_members_of(ti, std::meta::access_context::unchecked())) try {
        if (is_private(member)) throw std::meta::exception("member cannot be private", {});
        if (is_protected(member)) throw std::meta::exception("member cannot be protected", {});
        validate_arg(type_of(member));
      } catch (const std::meta::exception& e) {
        throw std::meta::exception(
          std::format("while validating member variable `{}`\n{}", display_string_of(member), e.what()), e.from(),
          e.where()
        );
      }
  } catch (const std::meta::exception& e) {
    throw std::meta::exception(
      std::format("while validating type `{}`\n{}", ivl::reflection::display_string_of(arg), e.what()), e.from(),
      e.where()
    );
  }
}

struct search_result_t {
  std::meta::info main_type;
  std::meta::info ivl_main_arg_type;
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
  if (params.size() != 1) throw std::meta::exception("unexpected number of arguments to `ivl_main` entry point", decl);

  res.main_type = ^^int(int, char**);
  res.ivl_main_arg_type = decay(type_of(params[0]));
  validate_arg(res.ivl_main_arg_type);
  res.emit_main = true;
  return res;
}

constexpr search_result_t search_result = find_main_declarations();

template <typename T>
void print_help_impl(std::string prefix) {
  std::println("{}", ivl::reflection::display_string_of(^^T));
  template for (constexpr auto mem :
                define_static_array(nonstatic_data_members_of(^^T, std::meta::access_context::unchecked()))) {
    if constexpr (is_class_type(type_of(mem)) && !ivl::reflection::is_child_of(type_of(mem), ^^std)) {
      print_help_impl<typename[:type_of(mem):]>(prefix + identifier_of(mem) + ".");
    } else {
      std::println("  --{}{}: {}", prefix, identifier_of(mem), ivl::reflection::display_string_of(type_of(mem)));
    }
  }
}

template <typename T>
void print_help() {
  std::println("  --help");
  std::println();
  print_help_impl<T>("");
}

template <typename arg_t>
int wrap_ivl_main(int argc, char** argv) {
  arg_t arg{};

  std::span<const char*> args((const char**)argv + 1, (const char**)argv + argc);

  for (auto arg : args)
    if (std::string_view(arg) == "--help") {
      print_help<arg_t>();
      return 1;
    }

  std::string_view context;
  auto take_arg = [&] {
    if (!args.empty()) {
      std::string_view ret = args[0];
      args = args.subspan(1);
      return ret;
    }

    std::println(stderr, "missing arguments while parsing {:?}", context);
    exit(1);
  };
  auto construct = [&]<typename T>(this const auto& self) -> T {
    if constexpr (^^T == ^^bool) return true;
    else if constexpr (is_integral_type(^^T) || is_floating_point_type(^^T)) {
      auto arg = take_arg();
      T value;
      auto ret = std::from_chars<T>(arg.data(), arg.data() + arg.size(), value);
      if (!ret || ret.ptr != arg.data() + arg.size()) {
        std::println(stderr, "bad value passed to argument {:?}: {:?}", context, arg);
        exit(1);
      }
      return value;
    } else if constexpr (has_template_arguments(^^T) && template_of(^^T) == ^^std::optional) {
      return self.template operator()<typename[:template_arguments_of(^^T)[0]:]>();
    } else if constexpr (ivl::meta::same_as_one_of<T, std::string, std::string_view, std::filesystem::path>) {
      return T(take_arg());
    } else {
      static_assert(false, std::format("type `{}` not implemented", display_string_of(^^T)));
    }
  };

  auto store = [&](std::string_view name) {
    template for (constexpr auto member : std::define_static_array(
                    nonstatic_data_members_of(^^arg_t, std::meta::access_context::unchecked())
                  )) {
      if (name.substr(2) == identifier_of(member)) {
        arg.[:member:] = construct.template operator()<typename[:type_of(member):]>();
        return;
      }
    }
    std::println(stderr, "unrecognized argument: {:?}", name);
    exit(1);
  };

  while (!args.empty()) {
    auto arg = take_arg();
    context = arg;
    if (!arg.starts_with("--")) {
      std::println(stderr, "invalid argument name (missing \"--\" prefix): {:?}", arg);
      exit(1);
    }
    store(arg);
  }

  return ivl_main(arg);
}

template <bool use_ivl, typename arg_t>
int main_template(int argc, char** argv) {
  if constexpr (use_ivl) {
    return wrap_ivl_main<arg_t>(argc, argv);
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
    ivl::main_synthesis::search_result.emit_main,
    typename[:ivl::main_synthesis::search_result.emit_main ? ivl::main_synthesis::search_result.ivl_main_arg_type : ^^
             void:]>(argc, argv);
}
