#include <experimental/meta>
#include <type_traits>
#include <concepts>
#include <cassert>

namespace ivl {

  consteval bool decays_to_util(std::meta::info T, std::meta::info C){
    assert(std::meta::is_concept(C));
    // assert(T == ^int);
    assert(C == ^std::copyable);
    // assert(false);
    auto I = std::meta::substitute(C, {T});
    return std::meta::extract<bool>(I);
  }

  template<typename T, std::meta::info C>
  constexpr bool decays_to_util2 = decays_to_util(^T, C);

  template<typename T, std::meta::info C>
  concept decays_to = decays_to_util2<T, C>;
  
} // namespace ivl

using X = int;
static_assert(ivl::decays_to_util(^X, ^std::copyable));

// template<decays_to<^std::copyable> T>
// auto f(T&& x);

static_assert(std::copyable<int>);
static_assert(ivl::decays_to_util(^int, ^std::copyable));
static_assert(ivl::decays_to_util2<int, ^std::copyable>);
static_assert(ivl::decays_to<int, ^std::copyable>);
// static_assert(decays_to<int&, ^std::copyable>);

// void use(){
//   int x;
//   f(x);
//   f(std::move(x));
// }
