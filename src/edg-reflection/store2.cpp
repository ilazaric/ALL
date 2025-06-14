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
void fill_vec_end(std::vector<void (*)(void*, void*)>&);

struct Callable {
  void*                               underlying;
  std::vector<void (*)(void*, void*)> invokers;
  using TL = TypeList<>;

  template <typename T>
  Callable(T&& underlying) : underlying(new std::remove_cvref_t<T>(std::forward<T>(underlying))) {
    fill_vec_end<std::remove_cvref_t<T>>(invokers);
  }

  void operator()(auto&& arg)
    requires((TL::push(std::vector {^decltype(arg)}), true))
  {
    auto idx = TL::find({^decltype(arg)});
    assert(idx < invokers.size());
    invokers[idx](underlying, &arg);
  }
};

void ender();
///////////// USER CODE //////////////
int main() {
  ender();
  Callable f = [](auto const& e) { std::cout << __PRETTY_FUNCTION__ << std::endl; };
  std::cerr << "invokers len: " << f.invokers.size() << std::endl;
  f(12);
  f(12);
  f(3.5);
  // f("abc");
  f = [](auto const& e) { std::cout << e << '\n'; };
  f(12);
  f(12);
  f(3.5);
  // f("abc");
}
///////////// USER CODE //////////////

template <std::size_t Idx, typename Fn>
void fill_vec(std::vector<void (*)(void*, void*)>& vec) {
  if constexpr (Idx != 0) {
    fill_vec<Idx - 1, Fn>(vec);
    vec.push_back([](void* a, void* b) {
      using ArgType = [:Callable::TL::get(Idx - 1)[0]:];
      (*reinterpret_cast<Fn*>(a))(
        static_cast<ArgType>(*reinterpret_cast<std::add_pointer_t<ArgType>>(b)));
    });
  }
}

template <typename Fn>
void fill_vec_end(std::vector<void (*)(void*, void*)>& vec) {
  fill_vec<Callable::TL::length(), Fn>(vec);
}

void ender() {
  std::cerr << "type list len: " << Callable::TL::length() << std::endl;
}
