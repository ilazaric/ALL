#pragma once

#include <type_traits>
#include <utility>

namespace ivl {

  template <typename>
  struct to_free_function_pointer_helper : std::false_type {};

// qualifiers to member functions are annoying
#define IVLGEN_PARTIAL(QUALS)                                                                                          \
  template <typename Class, typename Ret, typename... Args>                                                            \
  struct to_free_function_pointer_helper<Ret (Class::*)(Args...) QUALS> : std::true_type {                             \
    template <Ret (Class::*mfp)(Args...) QUALS>                                                                        \
    static Ret free_function(                                                                                          \
      std::conditional_t<std::is_reference_v<Class QUALS>, Class QUALS, std::add_lvalue_reference_t<Class QUALS>> arg, \
      Args... args                                                                                                     \
    ) {                                                                                                                \
      return (std::forward<decltype(arg)>(arg).*mfp)(std::forward<Args>(args)...);                                     \
    }                                                                                                                  \
  };

#define IVLGEN_CONST(QUALS)                                                                                            \
  IVLGEN_PARTIAL(QUALS)                                                                                                \
  IVLGEN_PARTIAL(const QUALS)

#define IVLGEN_VOLATILE(QUALS)                                                                                         \
  IVLGEN_CONST(QUALS)                                                                                                  \
  IVLGEN_CONST(volatile QUALS)

  IVLGEN_VOLATILE()
  IVLGEN_VOLATILE(&)
  IVLGEN_VOLATILE(&&)

#undef IVLGEN_PARTIAL
#undef IVLGEN_CONST
#undef IVLGEN_VOLATILE

  template <auto mfp>
  struct to_free_function_pointer {
    using helper = to_free_function_pointer_helper<decltype(mfp)>;
    static_assert(helper::value, "template argument not a member function pointer");
    static constexpr auto value = helper::template free_function<mfp>;
  };

  template <auto mfp>
  constexpr auto to_free_function_pointer_v = to_free_function_pointer<mfp>::value;

} // namespace ivl
