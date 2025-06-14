#include <any>
#include <cassert>
#include <experimental/meta>
#include <iostream>
#include <map>
#include <memory>
#include <ranges>
#include <span>
#include <vector>

#include "https://raw.githubusercontent.com/ilazaric/ALL/master/src/edg-reflection/incrementer.hpp"

template <typename = decltype([] {})>
struct TypeList {
private:
  using Inc = ivl::refl::Incrementer<>;

  // used for template param storage
  template <typename...>
  struct Types {};
  template <std::size_t>
  struct Index {};

  // used for stateful TMP
  template <std::size_t>
  struct IndexToTypes;
  template <typename...>
  struct TypesToIndex;

public:
  static constexpr std::size_t NOT_FOUND = std::size_t(-1);

  static consteval std::size_t length() { return Inc::get(); }

  static consteval std::vector<std::meta::info> get(std::size_t idx) {
    assert(idx < length());
    auto slot = substitute(^IndexToTypes, {
                                            std::meta::reflect_value(idx)});
    return template_arguments_of(type_of(nonstatic_data_members_of(slot)[0]));
  }

  static consteval std::size_t find(std::vector<std::meta::info> is) {
    auto slot = substitute(^TypesToIndex, is);
    if (is_incomplete_type(slot))
      return NOT_FOUND;
    return extract<std::size_t>(
      template_arguments_of(type_of(nonstatic_data_members_of(slot)[0]))[0]);
  }

  static consteval void push(std::vector<std::meta::info> is) {
    if (find(is) != NOT_FOUND)
      return;
    auto idx = Inc::get();
    Inc::advance();
    define_class(substitute(^IndexToTypes,
                            {
                              std::meta::reflect_value(idx)}),
                 {std::meta::nsdm_description(substitute(^Types, is))});
    define_class(
      substitute(^TypesToIndex, is),
      {std::meta::nsdm_description(substitute(^Index, {
                                                        std::meta::reflect_value(idx)}))});
  }
};

namespace type_list_test {
  using A = TypeList<>;
  consteval bool initialize() {
    A::push({^int});
    A::push({^char});
    A::push({^int&& });
    A::push({^double});
    return true;
  }
  static_assert(initialize());
  static_assert(A::length() == 4);
  static_assert(A::get(0)[0] == ^int);
  static_assert(A::get(1)[0] == ^char);
  static_assert(A::get(2)[0] == ^int&&);
  static_assert(A::get(3)[0] == ^double);
  static_assert(A::find({^int}) == 0);
  static_assert(A::find({^char}) == 1);
  static_assert(A::find({^int&& }) == 2);
  static_assert(A::find({^double}) == 3);
} // namespace type_list_test

template <typename Fn>
void fill_vec_end(std::vector<void*>&);

// dont worry about it, just abusing pack expansion
template <typename A, typename B>
using Drop = B;

// cv qualifiers returned in unerase_invoke() so hopefully good?
// i think we are allowed to remove cv as long as we return
// them before using the thingy
auto remove_cv_from_ptr(auto* ptr) {
  return const_cast<void*>(reinterpret_cast<const volatile void*>(ptr));
}

struct Callable {
  void* underlying;
  void (*destroyer)(void*);
  std::vector<void*> invokers;
  using TL = TypeList<>;

  Callable() : underlying(nullptr), destroyer(nullptr) {}

  template <typename T>
  Callable(T&& underlying)
      : underlying(new std::remove_cvref_t<T>(std::forward<T>(underlying))),
        destroyer([](void* ptr) { delete reinterpret_cast<std::remove_cvref_t<T>*>(ptr); }) {
    fill_vec_end<std::remove_cvref_t<T>>(invokers);
  }

  Callable(const Callable&) = delete;
  Callable(Callable&& o) : underlying(o.underlying), destroyer(o.destroyer), invokers(o.invokers) {
    o.underlying = nullptr;
    o.destroyer  = nullptr;
    o.invokers.clear();
  }

  Callable& operator=(const Callable&) = delete;
  Callable& operator=(Callable&& o) {
    if (this == &o)
      return *this;
    std::swap(underlying, o.underlying);
    std::swap(destroyer, o.destroyer);
    std::swap(invokers, o.invokers);
    return *this;
  }

  void operator()(auto&&... args)
    requires((TL::push(std::vector {^decltype(args)...}), true))
  {
    auto idx = TL::find({^decltype(args)...});
    assert(idx < invokers.size());
    auto type_erased_callable =
      reinterpret_cast<void (*)(void*, Drop<decltype(args), void*>...)>(invokers[idx]);
    type_erased_callable(underlying, remove_cv_from_ptr(&args)...);
  }

  ~Callable() {
    if (underlying != nullptr)
      destroyer(underlying);
  }
};

///////////// USER CODE //////////////
int main() {
  Callable f = [](auto const&...) { std::cout << __PRETTY_FUNCTION__ << std::endl; };
  std::cerr << "invokers len: " << f.invokers.size() << std::endl;
  f(12);
  f(12);
  f(3.5);
  f(1, 2, 3);
  f("abc");
  volatile int x = 12;
  f(x);
  f = [](auto const& e) { std::cout << e << '\n'; };
  f(12);
  f(12);
  f(3.5);
  f("abc");

  f = [](std::string s) { std::cout << "LOG " << s << std::endl; };
  f("abcd");
}
///////////// USER CODE //////////////

template <typename Fn, typename... Args>
void unerase_invoke(void* fn, Drop<Args, void*>... args) {
  auto* fptr = reinterpret_cast<Fn*>(fn);
  // this is to we can shove functional objects into Callable
  // that don't have the same sets of possible argument types
  // for example:
  // Callable a = [](int){};
  // Callable b = [](std::string){};
  // alternative (probably better) design would be to shove nullptr info invokers
  // if it is not callable, and assert in Callable::operator()
  if constexpr (requires {
                  (*fptr)(static_cast<Args>(*reinterpret_cast<std::add_pointer_t<Args>>(args))...);
                }) {
    (*fptr)(static_cast<Args>(*reinterpret_cast<std::add_pointer_t<Args>>(args))...);
  }
}

consteval std::vector<std::meta::info> prepend(std::meta::info i, std::vector<std::meta::info> is) {
  is.insert(is.begin(), i);
  return is;
}

template <std::size_t Idx, typename Fn>
void fill_vec(std::vector<void*>& vec) {
  if constexpr (Idx != 0) {
    fill_vec<Idx - 1, Fn>(vec);
    auto fptr = &[:substitute(^unerase_invoke, prepend(^Fn, Callable::TL::get(Idx - 1))):];
    vec.push_back(reinterpret_cast<void*>(fptr));
  }
}

template <typename Fn>
void fill_vec_end(std::vector<void*>& vec) {
  fill_vec<Callable::TL::length(), Fn>(vec);
}
