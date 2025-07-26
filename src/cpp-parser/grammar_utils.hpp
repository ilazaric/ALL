#pragma once

#include <concepts>
#include <expected>
#include <ivl/logger>
#include <ivl/util/fixed_string.hpp>
#include <ivl/util>
#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

struct NewLine;
template<typename> struct Entity;

namespace ivl::cppp::grammar {

  template<typename> struct Opt;
  template<typename, size_t = 1> struct List;
  template<typename...> struct And;
  template<typename...> struct Or;
  template<util::fixed_string> struct Terminal;
  template<util::fixed_string> struct Keyword;
  template<typename> struct UnimplementedTODO;

template <typename... Ts>
struct Overload : Ts... {
  using Ts::operator()...;
};
template <typename... Ts>
Overload(Ts...) -> Overload<Ts...>;

std::string X(size_t n){return std::string(n, '.');}

template <typename T>
concept Flat = requires { typename T::flat; };
  
const auto dump_grammar = Overload {
  []<typename E>(this const auto& self, const Entity<E>& e, size_t depth) {
    std::cerr << X(depth) << " -- entity(" << util::typestr<E>() << ")" << std::endl;
    if constexpr (Flat<E>) {
      self(e.data, depth + 1);
    } else {
      self(*e.data, depth + 1);
    }
  },
  []<typename... Ts>(this const auto& self, const And<Ts...>& a, size_t depth) {
    std::cerr << X(depth) << " -- conjunction" << std::endl;
    [&]<size_t... Idxs>(std::index_sequence<Idxs...>) {
      (self(std::get<Idxs>(a.data), depth + 1), ...);
    }(std::make_index_sequence<sizeof...(Ts)> {});
    std::cerr << X(depth) << " -- conjunction done" << std::endl;
  },
  [](std::monostate, size_t){},
  []<typename T>(this const auto& self, const Opt<T>& o, size_t depth){
    std::cerr << X(depth) << " -- opt(" << util::typestr<T>() << ")" << std::endl;
    if (o.data) self(*o.data, depth+1);
    else std::cerr << X(depth) << " -- empty" << std::endl;
  },
  []<typename T>(const UnimplementedTODO<T>&, size_t){},
  []<typename... Ts>(this const auto& self, const Or<Ts...>& o, size_t depth) {
    std::cerr << X(depth) << " -- variant entry " << o.data.index() << std::endl;
    std::visit([&](auto&& x) { return self(x, depth + 1); }, o.data);
  },
  []<typename T, size_t N>(this const auto& self, const List<T, N>& l, size_t depth) {
    std::cerr << X(depth) << " -- list(" << util::typestr<T>() << ", " << N << ")" << std::endl;
    for (auto&& el : l.data)
      self(el, depth + 1);
    std::cerr << X(depth) << " -- list done" << std::endl;
  },
  []<util::fixed_string Str>(this const auto& self, const Terminal<Str>& t, size_t depth) {
    std::cerr << X(depth) << " -- terminal(" << Str.view() << ")" << std::endl;
  },
  []<util::fixed_string Str>(this const auto& self, const Keyword<Str>& t, size_t depth) {
    std::cerr << X(depth) << " -- keyword(" << Str.view() << ")" << std::endl;
  },
  [](const NewLine&, size_t depth){
    std::cerr << X(depth) << " -- \\n" << std::endl;
  }
};

  template <typename T>
  struct Consumed {
    [[no_unique_address]] T      data;
    [[no_unique_address]] size_t consumed;
  };

  template <typename T>
  using Result = std::expected<Consumed<T>, std::string>;

  template <typename... Ts>
  struct Or {
    std::variant<std::monostate, Ts...> data;

    // xTODO: change, 0 matches = soft error, 1 match = success, 2+ = hard error
    static Result<Or> try_parse(std::string_view sv) {
      std::tuple<Result<Ts>...> candidates {Ts::try_parse(sv)...};
      bool                      found     = false;
      size_t                    found_idx = 0;
      Result<Or>                res {Consumed<Or> {}};
      [&]<size_t... Is>(std::index_sequence<Is...>) {
        (
          [&]<size_t I>() {
            if (!std::get<I>(candidates))
              return;
            if (found) {
              LOG("multiple matches");
              LOG(util::typestr<Or>());
              LOG(I);
              LOG(sv.substr(0, std::get<I>(candidates).value().consumed));
              dump_grammar(std::get<I>(candidates).value().data, 0);
              LOG(found_idx);
              LOG(sv.substr(0, res.value().consumed));
              dump_grammar(res.value().data, 0);
              LOG(sv);
              throw "ded";
            }
            found                 = true;
            found_idx = I;
            res.value().data.data = std::move(std::get<I>(candidates).value().data);
            res.value().consumed  = std::get<I>(candidates).value().consumed;
          }.template operator()<Is>(),
          ...);
      }(std::make_index_sequence<sizeof...(Ts)> {});
      if (!found)
        res = std::unexpected("no matches");
      return res;
    }
  };

  template <typename... Ts>
  struct And {
    std::tuple<Ts...> data;

    static Result<And> try_parse(std::string_view sv) {
      And         obj;
      bool        failed = false;
      std::string first_fail;
      size_t      total_consumed = 0;
      [&]<size_t... Is>(std::index_sequence<Is...>) {
        (
          [&]<size_t I>() {
            if (failed)
              return;
            auto x =
              std::tuple_element<I, std::tuple<Ts...>>::type::try_parse(sv.substr(total_consumed));
            if (!x) {
              failed     = true;
              first_fail = x.error();
              return;
            }
            std::get<I>(obj.data) = std::move(x.value().data);
            total_consumed += x.value().consumed;
          }.template operator()<Is>(),
          ...);
      }(std::make_index_sequence<sizeof...(Ts)> {});
      if (failed)
        return std::unexpected(first_fail);
      return Consumed {std::move(obj), total_consumed};
    }
  };

  template <typename T>
  struct Opt {
    std::optional<T> data;

    static Result<Opt> try_parse(std::string_view sv) {
      Opt    opt;
      size_t consumed = 0;
      auto   x        = T::try_parse(sv);
      if (x) {
        opt.data = std::move(x.value().data);
        consumed = x.value().consumed;
      }
      return Consumed {std::move(opt), consumed};
    }
  };

  template <typename T, size_t MinCount>
  struct List {
    std::vector<T> data;

    static Result<List> try_parse(std::string_view sv) {
      std::vector<T> data;
      size_t         consumed = 0;
      for (size_t i = 0;; ++i) {
        auto x = T::try_parse(sv.substr(consumed));
        if (x) {
          data.emplace_back(std::move(x.value().data));
          consumed += x.value().consumed;
          continue;
        }
        if (i < MinCount)
          return std::unexpected(x.error());
        break;
      }
      return Consumed {List {std::move(data)}, consumed};
    }
  };

  template <util::fixed_string Str>
  struct Terminal {
    static Result<Terminal> try_parse(std::string_view sv) {
      size_t cnt = 0;
      while (!sv.empty() && ::isspace(sv[0]) && sv[0] != '\n')
        ++cnt, sv.remove_prefix(1);
      if (!sv.starts_with(Str.view()))
        return std::unexpected("nope");
      return Consumed {Terminal {}, Str.size() + cnt};
    }
  };

  template <util::fixed_string Str>
  struct Keyword {
    static Result<Keyword> try_parse(std::string_view sv) {
      size_t cnt = 0;
      while (!sv.empty() && ::isspace(sv[0]) && sv[0] != '\n')
        ++cnt, sv.remove_prefix(1);
      if (!sv.starts_with(Str.view()))
        return std::unexpected("nope");
      return Consumed {Keyword {}, Str.size() + cnt};
    }
  };

  // #define ENTITY(name, ...)                                                                          \
//   struct name {                                                                                    \
//     std::unique_ptr<__VA_ARGS__> data;                                                             \
//                                                                                                    \
//     static Result<name> try_parse(std::string_view sv) {                                           \
//       auto x = __VA_ARGS__::try_parse(sv);                                                         \
//       if (!x)                                                                                      \
//         return std::unexpected(x.error());                                                         \
//       return Consumed {name {std::make_unique<__VA_ARGS__>(std::move(x.value().data))},            \
//                        x.value().consumed};                                                        \
//     }                                                                                              \
//   }

  template <typename = decltype([] {})>
  struct UnimplementedTODO {
    static Result<UnimplementedTODO> try_parse(std::string_view) {
      return std::unexpected("not implemented TODO");
    }
  };

  // #define FLAT_WRAP(name, ...)                                                                       \
//   struct name {                                                                                    \
//     __VA_ARGS__         data;                                                                      \
//     static Result<name> try_parse(std::string_view sv) {                                           \
//       auto r = __VA_ARGS__::try_parse(sv);                                                         \
//       if (!r)                                                                                      \
//         return std::unexpected(r.error());                                                         \
//       return Consumed {name {std::move(r.value().data)}, r.value().consumed};                      \
//     }                                                                                              \
//   }

} // namespace ivl::cppp::grammar
