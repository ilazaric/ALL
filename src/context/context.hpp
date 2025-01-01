#pragma once

#include <type_traits>
#include <cstddef>

namespace ivl::ctx {

  // default is `const T&`

  // represents a `T&`
  template<typename T> struct Mutable;

  // represents a `const T`
  template<typename T> struct Value;

  // so we can get<Tag>() to get the T
  template<typename Tag, typename T> struct Tagged;

  
  // dont look its ugly
  namespace detail {

    template<typename>
    struct IsOurType {
      static constexpr bool value = false;
    };

    template<typename T>
    struct IsOurType<Mutable<T>> {
      static constexpr bool value = true;
    };

    template<typename T>
    struct IsOurType<Value<T>> {
      static constexpr bool value = true;
    };

    template<typename T, typename U>
    struct IsOurType<Tagged<T, U>> {
      static constexpr bool value = true;
    };

    template<typename>
    struct IsTagged {
      static constexpr bool value = false;
    };

    template<typename T, typename U>
    struct IsTagged<Tagged<T, U>> {
      static constexpr bool value = true;
    };
    
    template<typename T>
    struct Type {
      static_assert(!IsOurType<T>::value, "This shouldn't ever trigger, yell at @ilazaric");
      using type = const T&;
    };

    template<typename T>
    struct Type<Mutable<T>> {
      static_assert(!IsOurType<T>::value, "You can't mix the ctx modifying types in this way");
      using type = T&;
    };

    template<typename T>
    struct Type<Value<T>> {
      static_assert(std::is_trivially_copyable_v<T>, "Your class is pretty complex, I don't think you want to pass it by value, are you sure a `const T&` is not appropriate?");
      static_assert(!IsOurType<T>::value, "You can't mix the ctx modifying types in this way");
      using type = T;
    };

    template<typename T, typename U>
    struct Type<Tagged<T, U>> {
      static_assert(!IsOurType<T>::value, "This just makes no sense at all");
      // U can be Value<_> or Mutable<_>
      static_assert(!IsTagged<U>::value, "Tagged<_, Tagged> makes no sense");
      using type = typename Type<U>::type;
    };

    template<typename T>
    struct Tag {
      static_assert(!IsTagged<T>::value, "Shouldn't trigger, yell at @ilazaric");
      using type = std::remove_cvref_t<typename Type<T>::type>;
    };
    
    template<typename T, typename U>
    struct Tag<Tagged<T, U>> {
      static_assert(!IsOurType<T>::value, "This just makes no sense at all");
      using type = T;
    };

    template<typename...>
    struct Tag2Type {
      static_assert(false, "This shouldn't get instantiated, yell at @ilazaric");
    };

    template<typename T, typename U, typename... Us>
    struct Tag2Type<T, U, Us...> {
      using type = typename std::conditional_t<std::is_same_v<T, typename Tag<U>::type>, Type<U>, Tag2Type<T, Us...>>::type;
    };

    template<typename...>
    struct Tag2Index {};
    template<typename T, typename U, typename... Us>
    struct Tag2Index<T, U, Us...> {
      static constexpr size_t value = std::conditional_t<std::is_same_v<T, typename Tag<U>::type>,
                                                         std::integral_constant<size_t, (size_t)-1>,
                                                         Tag2Index<T, Us...>>::value + 1;
    };

    template<typename... Ts>
    constexpr bool validate_tag_uniqueness(){
      size_t idx = 0;
      bool check = true;
      auto bla = [&] <typename T> {
        check &= idx++ == Tag2Index<typename Tag<T>::type, Ts...>::value;
      };
      (bla.template operator()<Ts>(), ...);
      return check;
    }

    template<typename T>
    struct SingleStorage {
      typename Type<T>::type data;

      SingleStorage(typename Type<T>::type data) : data(data){}

      template<typename U>
      requires std::is_same_v<typename Tag<T>::type, U>
      typename Type<T>::type get(){return data;}
    };

    template<typename...>
    struct Storage;

    template<>
    struct Storage<> {
      template<typename>
      requires false
      void get();
    };

    template<typename T, typename... Ts>
    struct Storage<T, Ts...> : private SingleStorage<T>, private Storage<Ts...> {
      Storage(typename Type<T>::type head, typename Type<Ts>::type... tail) : SingleStorage<T>(head), Storage<Ts...>(tail...){}

      using SingleStorage<T>::get;
      using Storage<Ts...>::get;
    };
    
  } // namespace detail
  // you can look now

  template<typename... Ts>
  struct Context {
    static_assert(detail::validate_tag_uniqueness<Ts...>());

    detail::Storage<Ts...> storage;

    Context(typename detail::Type<Ts>::type... args) : storage(args...){}

    template<typename... Us>
    requires (sizeof...(Us) != sizeof...(Ts) || (!std::is_same_v<Us, Ts> || ...))
    && (std::is_same_v<typename detail::Type<Ts>::type, typename detail::Tag2Type<typename detail::Tag<Ts>::type, Us...>::type> && ...)
    Context(Context<Us...> ctx) : Context(ctx.template get<typename detail::Tag<Ts>::type>()...){}

    Context(const Context&) = default;
    Context(Context&&) = default;

    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;

    template<typename T>
    typename detail::Tag2Type<T, Ts...>::type get(){
      return storage.template get<T>();
    }

    // template<typename... Us>
    // requires (std::is_same_v<typename detail::Type<Us>::type, typename detail::Tag2Type<typename detail::Tag<Us>::type, Ts...>::type> && ...)
    // operator Context<Us...>(){
    //   return {this->get<typename detail::Tag<Us>::type>()...};
    // };
  };

  template<typename... Ts>
  Context(Ts&&...) -> Context<std::remove_cvref_t<Ts>...>;

} // namespace ivl::ctx
