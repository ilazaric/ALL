#include <ivl/fs/fileview.hpp>
#include <ivl/logger>
#include <ivl/util>
#include <string>
#include <vector>

#include "grammar_utils.hpp"
using namespace ivl;
using namespace cppp;
using namespace grammar;

template <typename>
struct Entity;

#define ENTITY(name, ...) struct name
#include "generated.hpp"
#undef ENTITY

#define FLAT_WRAP(name, ...)                                                                                           \
  struct name {                                                                                                        \
    using type = __VA_ARGS__;                                                                                          \
    using flat = void;                                                                                                 \
  }

// missing entities
FLAT_WRAP(entity_import_keyword, Keyword<"import">);
FLAT_WRAP(entity_module_keyword, Keyword<"module">);
FLAT_WRAP(entity_export_keyword, Keyword<"export">);

// overriden entities
// FLAT_WRAP(entity_identifier_start, Entity<entity_nondigit>);
// TODO: no space
// FLAT_WRAP(entity_identifier_continue, Or<Entity<entity_digit>, Entity<entity_nondigit>>);

// struct entity_identifier {
//   using type = And<Entity<entity_identifier_start>, List<Entity<entity_identifier_continue>, 0>>;
// };

struct NewLine {
  static Result<NewLine> try_parse(std::string_view sv) {
    size_t cnt = 0;
    while (!sv.empty() && sv[0] != '\n' && isspace(sv[0]))
      ++cnt, sv.remove_prefix(1);
    if (sv.empty() || sv[0] != '\n') return std::unexpected("not newline");
    return Consumed{NewLine{}, cnt + 1};
  }
};

struct entity_new_line {
  using flat = void;
  using type = NewLine;
};

#define ENTITY(name, ...)                                                                                              \
  struct name {                                                                                                        \
    using type = __VA_ARGS__;                                                                                          \
  }
#include "generated.hpp"
#undef ENTITY

// #define ENTITY(name, ...)                                                                          \
//   struct name {                                                                                    \
//     std::unique_ptr<__VA_ARGS__> data;                                                             \
//     static Result<name>          try_parse(std::string_view);                                      \
//   }
// #include "generated.hpp"
// #undef ENTITY

// #define ENTITY(name, ...)                                                                          \
//   Result<name> name::try_parse(std::string_view sv) {                                              \
//     auto x = __VA_ARGS__::try_parse(sv);                                                           \
//     if (!x)                                                                                        \
//       return std::unexpected(x.error());                                                           \
//     return Consumed {name {std::make_unique<__VA_ARGS__>(std::move(x.value().data))},              \
//                      x.value().consumed};                                                          \
//   }
// #include "generated.hpp"
// #undef ENTITY

template <typename E>
  requires(!Flat<E>)
struct Entity<E> {
  std::unique_ptr<typename E::type> data;

  static Result<Entity> try_parse(std::string_view sv) {
    auto x = E::type::try_parse(sv);
    if (!x) return std::unexpected(x.error());
    return Consumed{Entity{std::make_unique<typename E::type>(std::move(x.value().data))}, x.value().consumed};
  }
};

template <typename E>
  requires(Flat<E>)
struct Entity<E> {
  E::type data;

  static Result<Entity> try_parse(std::string_view sv) {
    auto x = E::type::try_parse(sv);
    if (!x) return std::unexpected(x.error());
    return Consumed{Entity{std::move(x.value().data)}, x.value().consumed};
  }
};

int main(int argc, char** argv) {
  assert(argc == 2);
  ivl::fs::FileView fv{argv[1]};
  auto              sv = fv.as_string_view();
  LOG(sv);
  auto res = Entity<entity_preprocessing_file>::try_parse(sv);

  LOG((bool)res);
  if (res) {
    LOG(res.value().consumed);
    dump_grammar(res.value().data, 0);
  } else LOG(res.error());
}
