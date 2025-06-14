#pragma once

#include <concepts>
#include <expected>
#include <memory>
#include <optional>
#include <type_traits>

struct Hello;
struct World;

namespace ivl::cppp::grammar {

  template <typename T>
  struct Consumed {
    T      data;
    size_t consumed;
  };

  template <typename T>
  using Result = std::expected<Consumed<T>, std::string>;

  template <typename... Ts>
  struct Or {
    std::variant<std::monostate, Ts...> data;

    static Result<Or> try_parse(std::string_view sv) {
      std::tuple<Result<Ts>...> candidates {Ts::try_parse(sv)...};
      bool                      found = false;
      Result<Or>                res {Consumed<Or> {}};
      [&]<size_t... Is>(std::index_sequence<Is...>) {
        (
          [&]<size_t I>() {
            if (!std::get<I>(candidates))
              return;
            if (found)
              return;
            found                 = true;
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

#define ENTITY(name, ...)                                                                          \
  struct name {                                                                                    \
    std::unique_ptr<__VA_ARGS__> data;                                                             \
                                                                                                   \
    static Result<name> try_parse(std::string_view sv) {                                           \
      auto x = __VA_ARGS__::try_parse(sv);                                                         \
      if (!x)                                                                                      \
        return std::unexpected(x.error());                                                         \
      return Consumed {name {std::make_unique<__VA_ARGS__>(std::move(x.value().data))},            \
                       x.value().consumed};                                                        \
    }                                                                                              \
  }

  struct UnimplementedTODO {
    static Result<UnimplementedTODO> try_parse(std::string_view) {
      return std::unexpected("not implemented TODO");
    }
  };

} // namespace ivl::cppp::grammar
