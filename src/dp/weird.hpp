#pragma once

/*

  when writing a dynamic the following is important:
  1) it should be performant (caching)
  2) it should be easy to write/read/wtv
  3) it should be easy to debug

  for 3 it would be useful if we could dump
  the expressions that were involved in
  computing the dp
  gonna need a lot of type erasure

  for 1 we would either need to know
  a topological sort or test if
  cache was set
  both sound kinda stupid
  first one is minimum overhead
  second one is simpler? to implement
  first one might be kinda hard,
  would probably need tuple apply,
  which might not be trivial to inline?
  maybe no ...

  ```
  auto expression(auto&& Getter, <dp-params>){...}
  ```
  ^ cost of param `Getter` ?

  cache options:
  mdvec
  hash map
  should be able to pick and configure

  if i added a `template<typename RT>` to `expression`
  it could improve debugging
  it would also worsen? implementing? maybe?
  maybe no

  how would multiple dps interact?
  oof
  tough
  maybe tags?

  dps interacting usually looks like:

  T dp1(T, T, ...);
  T dp2(T, T, ...);

  T dp1(T, T, ...){
    ...
    dp2(...);
    ...
  }

  T dp2(T, T, ...){
    ...
    dp1(...);
    ...
  }

  how to do something like ^ ?

  we would probably also prefer both dps "contained"
  in single class ?
  naming a bit of an issue ?

 */

#include <map>
#include <stdexcept>

namespace ivl::dp {

  struct normal_tag_type {};
  constexpr normal_tag_type normal {};

  template <std::size_t Depth>
  struct debug_tag_type {};
  template <std::size_t Depth = 1>
  constexpr debug_tag_type<Depth> debug {};

  template <typename RT, typename... Ts>
  struct DefaultCache {
    using KeyType   = std::tuple<Ts...>;
    using ValueType = RT;

    std::map<KeyType, ValueType> cache;

    // this is shitty for expensive types
    // TODO ^
    // can probably be made better with tuples of
    // references and a transparent comparator
    bool contains(Ts... args) const { return cache.contains(KeyType {std::move(args)...}); }

    RT& set(Ts... args, RT value) {
      auto [it, status] = cache.try_emplace(KeyType {std::move(args)...}, std::move(value));
      if (!status)
        throw std::runtime_error("DefaultCache: tried to set twice");
      return it->second;
    }

    const RT& get(Ts... args) const { return cache.at(KeyType {std::move(args)...}); }
  };

  template <typename T>
  struct DebugExpression {
    // TODO

    T           value;
    std::string expression;

    template <typename TT>
    explicit DebugExpression(TT arg) : value(arg) {}

    operator T() const { return value; }
  };

  template <typename T, typename Shape, template <typename...> typename Cache = DefaultCache>
  struct Helper;

  template <typename T, typename ShapeRT, typename... ShapeTs,
            template <typename...> typename Cache>
  struct Helper<T, ShapeRT(ShapeTs...), Cache> {
    Cache<ShapeRT, ShapeTs...> cache;

    T& get_parent() { return *static_cast<T*>(this); }

    ShapeRT compute(ShapeTs... args, normal_tag_type tag) {
      if (cache.contains(args...))
        return cache.get(args...);
      ShapeRT ret = get_parent().recursive_expression(args..., tag);
      cache.set(args..., ret);
      return ret;
    }

    template <std::size_t Depth>
    DebugExpression<ShapeRT> compute(ShapeTs... args, debug_tag_type<Depth>) {
      using next_tag_type =
        std::conditional_t<Depth == 0, normal_tag_type, debug_tag_type<Depth - 1>>;
      return get_parent().recursive_expression(args..., next_tag_type {});
    }
  };

} // namespace ivl::dp
