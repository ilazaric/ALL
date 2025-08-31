#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace ivl::ctx {

  // default is `const T&`

  // represents a `T&`
  template <typename T>
  struct Mutable;

  // represents a `const T`
  template <typename T>
  struct Value;

  // so we can get<Tag>() to get the T
  template <typename Tag, typename T>
  struct Tagged;

  // dont look its ugly
  namespace detail {

    template <typename>
    struct IsOurType {
      static constexpr bool value = false;
    };

    template <typename T>
    struct IsOurType<Mutable<T>> {
      static constexpr bool value = true;
    };

    template <typename T>
    struct IsOurType<Value<T>> {
      static constexpr bool value = true;
    };

    template <typename T, typename U>
    struct IsOurType<Tagged<T, U>> {
      static constexpr bool value = true;
    };

    template <typename>
    struct IsTagged {
      static constexpr bool value = false;
    };

    template <typename T, typename U>
    struct IsTagged<Tagged<T, U>> {
      static constexpr bool value = true;
    };

    template <typename T>
    struct Type {
      static_assert(!IsOurType<T>::value, "This shouldn't ever trigger, yell at @ilazaric");
      using type = const T&;
    };

    template <typename T>
    struct Type<Mutable<T>> {
      // TODO: better msg
      static_assert(!IsOurType<T>::value, "You can't mix the ctx modifying types in this way");
      using type = T&;
    };

    template <typename T>
    struct Type<Value<T>> {
      // this is before is_trivially_copyable bc they are incomplete
      // TODO: better msg
      static_assert(!IsOurType<T>::value, "You can't mix the ctx modifying types in this way");
      static_assert(
        std::is_trivially_copyable_v<T>,
        "Your class is pretty complex, I don't think you want to pass it by value, are "
        "you sure a `const T&` is not appropriate?"
      );
      using type = T;
    };

    template <typename T, typename U>
    struct Type<Tagged<T, U>> {
      // TODO: better msg
      static_assert(!IsOurType<T>::value, "This just makes no sense at all");
      // U can be Value<_> or Mutable<_> (or regular)
      static_assert(!IsTagged<U>::value, "Tagged<_, Tagged> makes no sense");
      using type = typename Type<U>::type;
    };

    template <typename T>
    struct Tag {
      static_assert(!IsTagged<T>::value, "Shouldn't trigger, yell at @ilazaric");
      using type = std::remove_cvref_t<typename Type<T>::type>;
    };

    template <typename T, typename U>
    struct Tag<Tagged<T, U>> {
      static_assert(!IsOurType<T>::value, "This just makes no sense at all");
      using type = T;
    };

    template <typename...>
    struct Tag2Type {
      using type = void; // means "didn't find it :("
    };

    template <typename T, typename U, typename... Us>
    struct Tag2Type<T, U, Us...> {
      using type =
        typename std::conditional_t<std::is_same_v<T, typename Tag<U>::type>, Type<U>, Tag2Type<T, Us...>>::type;
    };

    template <typename...>
    struct Tag2Index {
      static constexpr size_t value = (size_t)-1; // means "didn't find it :("
    };

    template <typename T, typename U, typename... Us>
    struct Tag2Index<T, U, Us...> {
      static constexpr size_t value = std::is_same_v<T, typename Tag<U>::type> ? 0 : Tag2Index<T, Us...>::value + 1 ?: (size_t)-1;
    };

    // TODO: revisit this
    template <typename... Ts>
    constexpr bool validate_tag_uniqueness() {
      size_t idx   = 0;
      bool   check = true;
      auto   bla   = [&]<typename T> { check &= idx++ == Tag2Index<typename Tag<T>::type, Ts...>::value; };
      (bla.template operator()<Ts>(), ...);
      return check;
    }

    template <typename T>
    struct SingleStorage {
      typename Type<T>::type data;

      SingleStorage(typename Type<T>::type data) : data(data) {}

      // TODO: this should be simpler, revisit when you have buildtime benchmarks
      template <typename U>
        requires std::is_same_v<typename Tag<T>::type, U>
      typename Type<T>::type get() {
        return data;
      }
    };

    template <typename...>
    struct Storage;

    template <>
    struct Storage<> {
      template <typename>
        requires false
      void get();
    };

    template <typename T, typename... Ts>
    struct Storage<T, Ts...> : SingleStorage<T>, Storage<Ts...> {
      // TODO: can we do better?
      Storage(typename Type<T>::type head, typename Type<Ts>::type... tail)
          : SingleStorage<T>(head), Storage<Ts...>(tail...) {}

      using SingleStorage<T>::get;
      using Storage<Ts...>::get;
    };

  } // namespace detail
  // you can look now

  template <typename... Ts>
  struct Context {
    // TODO: better error msg
    static_assert(detail::validate_tag_uniqueness<Ts...>());

    detail::Storage<Ts...> storage;

    Context(typename detail::Type<Ts>::type... args) : storage(args...) {}

    template <typename... Us>
      requires(sizeof...(Us) != sizeof...(Ts) || (!std::is_same_v<Us, Ts> || ...)) &&
              (std::is_same_v<
                 typename detail::Type<Ts>::type,
                 typename detail::Tag2Type<typename detail::Tag<Ts>::type, Us...>::type> &&
               ...)
    Context(Context<Us...> ctx) : Context(ctx.template get<typename detail::Tag<Ts>::type>()...) {}

    // TODO: want better constructors
    // Context(ctx1, ctx2, bla, mutable(foo), ...)

    Context(const Context&) = default;
    Context(Context&&)      = default;

    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&)      = delete;

    // auto& ref = ctx.get<T>();
    template <typename T>
    typename detail::Tag2Type<T, Ts...>::type get() {
      return storage.template get<T>();
    }

    // auto [foo, bar] = ctx.get<Foo, Bar>();
    template <typename... Us>
      requires(sizeof...(Us) >= 2)
    auto get() {
      return std::tuple<typename detail::Tag2Type<Us, Ts...>::type...>{get<Us>()...};
    }

    // TODO: put()

    // template<typename... Us>
    // requires (std::is_same_v<typename detail::Type<Us>::type, typename detail::Tag2Type<typename
    // detail::Tag<Us>::type, Ts...>::type> && ...) operator Context<Us...>(){
    //   return {this->get<typename detail::Tag<Us>::type>()...};
    // };
  };

  // TODO: want different stuff for rvalue garbage
  // if you give me an rvalue, i only work with rvalue ctxs
  // think about this more
  template <typename... Ts>
  Context(Ts&&...) -> Context<std::remove_cvref_t<Ts>...>;

} // namespace ivl::ctx
