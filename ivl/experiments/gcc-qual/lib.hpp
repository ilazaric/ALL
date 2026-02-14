#include <meta>
#include <print>

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
  if (params.size() != 1)
    throw std::meta::exception("unexpected number of arguments to `ivl_main` entry point", decl);
  
  res.main_type = ^^int(int, char**);
  res.ivl_main_arg_type = decay(type_of(params[0]));
  res.emit_main = true;
  return res;
}

constexpr search_result_t search_result = find_main_declarations();

[: search_result.main_type :] main;

template<typename arg_t>
int wrap_ivl_main(int argc, char** argv) {
  arg_t arg;

  auto store = [&](std::string_view name, std::string_view value) {
    template for (constexpr auto member : std::define_static_array(nonstatic_data_members_of(^^arg_t, std::meta::access_context::unchecked()))) {
      if (name == identifier_of(member)) {
        arg. [: member :] = value;
        return;
      }
    }
    std::println(stderr, "unrecognized argument name: {:?}", name);
    exit(1);
  };

  for (int i = 1; i < argc; ++i) {
    std::string_view arg(argv[i]);
    if (!arg.starts_with("--")) {
      std::println(stderr, "invalid argument name (missing \"--\" prefix): {:?}", arg);
      exit(1);
    }
    if (i + 1 == argc) {
      std::println(stderr, "missing value for argument {:?}", arg);
      exit(1);
    }
    store(arg.substr(2), argv[i + 1]);
    ++i;
  }
    
  return ivl_main(arg);
}

template<bool use_ivl, typename arg_t>
int main_template(int argc, char** argv) {
  if constexpr (use_ivl) {
    return wrap_ivl_main<arg_t>(argc, argv);
  } else {
    return 0;
  }
}

namespace {
namespace hide_decl {
int main(int, char**);
} // namespace hide_decl
} // namespace

int [: search_result.emit_main ? ^^:: : ^^hide_decl :] :: main(int argc, char** argv) {
  return main_template<
    search_result.emit_main,
    typename [: search_result.emit_main ? search_result.ivl_main_arg_type : ^^void :]
  >(argc, argv);
}
