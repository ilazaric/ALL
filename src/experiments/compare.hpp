// -*- C++ -*- operator<=> three-way comparison support.

// Copyright (C) 2019-2023 Free Software Foundation, Inc.
//
// This file is part of GCC.
//
// GCC is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// GCC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/** @file compare
 *  This is a Standard C++ Library header.
 */

#ifndef _COMPARE
#define _COMPARE

#pragma GCC system_header

#if __cplusplus > 201703L && __cpp_impl_three_way_comparison >= 201907L

#include <concepts>

#if __cpp_lib_concepts
#define __cpp_lib_three_way_comparison 201907L
#endif

namespace std _GLIBCXX_VISIBILITY(default) {
  // [cmp.categories], comparison category types

  namespace __cmp_cat {
    using type = signed char;

    enum class _Ord : type { equivalent = 0, less = -1, greater = 1 };

    enum class _Ncmp : type { _Unordered = 2 };

    struct __unspec {
      constexpr __unspec(__unspec*) noexcept {}
    };
  } // namespace __cmp_cat

  class partial_ordering {
    // less=0xff, equiv=0x00, greater=0x01, unordered=0x02
    __cmp_cat::type _M_value;

    constexpr explicit partial_ordering(__cmp_cat::_Ord __v) noexcept
        : _M_value(__cmp_cat::type(__v)) {}

    constexpr explicit partial_ordering(__cmp_cat::_Ncmp __v) noexcept
        : _M_value(__cmp_cat::type(__v)) {}

    friend class weak_ordering;
    friend class strong_ordering;

  public:
    // valid values
    static const partial_ordering less;
    static const partial_ordering equivalent;
    static const partial_ordering greater;
    static const partial_ordering unordered;

    // enumerators defined for better warnings
    enum class switch_enabler : __cmp_cat::type {
      less       = static_cast<__cmp_cat::type>(__cmp_cat::_Ord::less),
      equivalent = static_cast<__cmp_cat::type>(__cmp_cat::_Ord::equivalent),
      greater    = static_cast<__cmp_cat::type>(__cmp_cat::_Ord::greater),
      unordered  = static_cast<__cmp_cat::type>(__cmp_cat::_Ncmp::_Unordered)
    };

    constexpr operator switch_enabler() const { return static_cast<switch_enabler>(_M_value); }

    // comparisons
    [[nodiscard]]
    friend constexpr bool operator==(partial_ordering __v, __cmp_cat::__unspec) noexcept {
      return __v._M_value == 0;
    }

    [[nodiscard]]
    friend constexpr bool operator==(partial_ordering, partial_ordering) noexcept = default;

    [[nodiscard]]
    friend constexpr bool operator<(partial_ordering __v, __cmp_cat::__unspec) noexcept {
      return __v._M_value == -1;
    }

    [[nodiscard]]
    friend constexpr bool operator>(partial_ordering __v, __cmp_cat::__unspec) noexcept {
      return __v._M_value == 1;
    }

    [[nodiscard]]
    friend constexpr bool operator<=(partial_ordering __v, __cmp_cat::__unspec) noexcept {
      return __v._M_value <= 0;
    }

    [[nodiscard]]
    friend constexpr bool operator>=(partial_ordering __v, __cmp_cat::__unspec) noexcept {
      return __cmp_cat::type(__v._M_value & 1) == __v._M_value;
    }

    [[nodiscard]]
    friend constexpr bool operator<(__cmp_cat::__unspec, partial_ordering __v) noexcept {
      return __v._M_value == 1;
    }

    [[nodiscard]]
    friend constexpr bool operator>(__cmp_cat::__unspec, partial_ordering __v) noexcept {
      return __v._M_value == -1;
    }

    [[nodiscard]]
    friend constexpr bool operator<=(__cmp_cat::__unspec, partial_ordering __v) noexcept {
      return __cmp_cat::type(__v._M_value & 1) == __v._M_value;
    }

    [[nodiscard]]
    friend constexpr bool operator>=(__cmp_cat::__unspec, partial_ordering __v) noexcept {
      return 0 >= __v._M_value;
    }

    [[nodiscard]]
    friend constexpr partial_ordering operator<=>(partial_ordering __v,
                                                  __cmp_cat::__unspec) noexcept {
      return __v;
    }

    [[nodiscard]]
    friend constexpr partial_ordering operator<=>(__cmp_cat::__unspec,
                                                  partial_ordering __v) noexcept {
      if (__v._M_value & 1)
        return partial_ordering(__cmp_cat::_Ord(-__v._M_value));
      else
        return __v;
    }
  };

  // valid values' definitions
  inline constexpr partial_ordering partial_ordering::less(__cmp_cat::_Ord::less);

  inline constexpr partial_ordering partial_ordering::equivalent(__cmp_cat::_Ord::equivalent);

  inline constexpr partial_ordering partial_ordering::greater(__cmp_cat::_Ord::greater);

  inline constexpr partial_ordering partial_ordering::unordered(__cmp_cat::_Ncmp::_Unordered);

  class weak_ordering {
    __cmp_cat::type _M_value;

    constexpr explicit weak_ordering(__cmp_cat::_Ord __v) noexcept
        : _M_value(__cmp_cat::type(__v)) {}

    friend class strong_ordering;

  public:
    // valid values
    static const weak_ordering less;
    static const weak_ordering equivalent;
    static const weak_ordering greater;

    // enumerators defined for better warnings
    enum class switch_enabler : __cmp_cat::type {
      less       = static_cast<__cmp_cat::type>(__cmp_cat::_Ord::less),
      equivalent = static_cast<__cmp_cat::type>(__cmp_cat::_Ord::equivalent),
      greater    = static_cast<__cmp_cat::type>(__cmp_cat::_Ord::greater)
    };

    constexpr operator switch_enabler() const { return static_cast<switch_enabler>(_M_value); }

    [[nodiscard]]
    constexpr operator partial_ordering() const noexcept {
      return partial_ordering(__cmp_cat::_Ord(_M_value));
    }

    // comparisons
    [[nodiscard]]
    friend constexpr bool operator==(weak_ordering __v, __cmp_cat::__unspec) noexcept {
      return __v._M_value == 0;
    }

    [[nodiscard]]
    friend constexpr bool operator==(weak_ordering, weak_ordering) noexcept = default;

    [[nodiscard]]
    friend constexpr bool operator<(weak_ordering __v, __cmp_cat::__unspec) noexcept {
      return __v._M_value < 0;
    }

    [[nodiscard]]
    friend constexpr bool operator>(weak_ordering __v, __cmp_cat::__unspec) noexcept {
      return __v._M_value > 0;
    }

    [[nodiscard]]
    friend constexpr bool operator<=(weak_ordering __v, __cmp_cat::__unspec) noexcept {
      return __v._M_value <= 0;
    }

    [[nodiscard]]
    friend constexpr bool operator>=(weak_ordering __v, __cmp_cat::__unspec) noexcept {
      return __v._M_value >= 0;
    }

    [[nodiscard]]
    friend constexpr bool operator<(__cmp_cat::__unspec, weak_ordering __v) noexcept {
      return 0 < __v._M_value;
    }

    [[nodiscard]]
    friend constexpr bool operator>(__cmp_cat::__unspec, weak_ordering __v) noexcept {
      return 0 > __v._M_value;
    }

    [[nodiscard]]
    friend constexpr bool operator<=(__cmp_cat::__unspec, weak_ordering __v) noexcept {
      return 0 <= __v._M_value;
    }

    [[nodiscard]]
    friend constexpr bool operator>=(__cmp_cat::__unspec, weak_ordering __v) noexcept {
      return 0 >= __v._M_value;
    }

    [[nodiscard]]
    friend constexpr weak_ordering operator<=>(weak_ordering __v, __cmp_cat::__unspec) noexcept {
      return __v;
    }

    [[nodiscard]]
    friend constexpr weak_ordering operator<=>(__cmp_cat::__unspec, weak_ordering __v) noexcept {
      return weak_ordering(__cmp_cat::_Ord(-__v._M_value));
    }
  };

  // valid values' definitions
  inline constexpr weak_ordering weak_ordering::less(__cmp_cat::_Ord::less);

  inline constexpr weak_ordering weak_ordering::equivalent(__cmp_cat::_Ord::equivalent);

  inline constexpr weak_ordering weak_ordering::greater(__cmp_cat::_Ord::greater);

  class strong_ordering {
    __cmp_cat::type _M_value;

    constexpr explicit strong_ordering(__cmp_cat::_Ord __v) noexcept
        : _M_value(__cmp_cat::type(__v)) {}

  public:
    // valid values
    static const strong_ordering less;
    static const strong_ordering equal;
    static const strong_ordering equivalent;
    static const strong_ordering greater;

    // enumerators defined for better warnings
    enum class switch_enabler : __cmp_cat::type {
      less       = static_cast<__cmp_cat::type>(__cmp_cat::_Ord::less),
      equal      = static_cast<__cmp_cat::type>(__cmp_cat::_Ord::equivalent),
      equivalent = static_cast<__cmp_cat::type>(__cmp_cat::_Ord::equivalent),
      greater    = static_cast<__cmp_cat::type>(__cmp_cat::_Ord::greater)
    };

    constexpr operator switch_enabler() const { return static_cast<switch_enabler>(_M_value); }

    [[nodiscard]]
    constexpr operator partial_ordering() const noexcept {
      return partial_ordering(__cmp_cat::_Ord(_M_value));
    }

    [[nodiscard]]
    constexpr operator weak_ordering() const noexcept {
      return weak_ordering(__cmp_cat::_Ord(_M_value));
    }

    // comparisons
    [[nodiscard]]
    friend constexpr bool operator==(strong_ordering __v, __cmp_cat::__unspec) noexcept {
      return __v._M_value == 0;
    }

    [[nodiscard]]
    friend constexpr bool operator==(strong_ordering, strong_ordering) noexcept = default;

    [[nodiscard]]
    friend constexpr bool operator<(strong_ordering __v, __cmp_cat::__unspec) noexcept {
      return __v._M_value < 0;
    }

    [[nodiscard]]
    friend constexpr bool operator>(strong_ordering __v, __cmp_cat::__unspec) noexcept {
      return __v._M_value > 0;
    }

    [[nodiscard]]
    friend constexpr bool operator<=(strong_ordering __v, __cmp_cat::__unspec) noexcept {
      return __v._M_value <= 0;
    }

    [[nodiscard]]
    friend constexpr bool operator>=(strong_ordering __v, __cmp_cat::__unspec) noexcept {
      return __v._M_value >= 0;
    }

    [[nodiscard]]
    friend constexpr bool operator<(__cmp_cat::__unspec, strong_ordering __v) noexcept {
      return 0 < __v._M_value;
    }

    [[nodiscard]]
    friend constexpr bool operator>(__cmp_cat::__unspec, strong_ordering __v) noexcept {
      return 0 > __v._M_value;
    }

    [[nodiscard]]
    friend constexpr bool operator<=(__cmp_cat::__unspec, strong_ordering __v) noexcept {
      return 0 <= __v._M_value;
    }

    [[nodiscard]]
    friend constexpr bool operator>=(__cmp_cat::__unspec, strong_ordering __v) noexcept {
      return 0 >= __v._M_value;
    }

    [[nodiscard]]
    friend constexpr strong_ordering operator<=>(strong_ordering __v,
                                                 __cmp_cat::__unspec) noexcept {
      return __v;
    }

    [[nodiscard]]
    friend constexpr strong_ordering operator<=>(__cmp_cat::__unspec,
                                                 strong_ordering __v) noexcept {
      return strong_ordering(__cmp_cat::_Ord(-__v._M_value));
    }
  };

  // valid values' definitions
  inline constexpr strong_ordering strong_ordering::less(__cmp_cat::_Ord::less);

  inline constexpr strong_ordering strong_ordering::equal(__cmp_cat::_Ord::equivalent);

  inline constexpr strong_ordering strong_ordering::equivalent(__cmp_cat::_Ord::equivalent);

  inline constexpr strong_ordering strong_ordering::greater(__cmp_cat::_Ord::greater);

  // named comparison functions
  [[nodiscard]]
  constexpr bool is_eq(partial_ordering __cmp) noexcept {
    return __cmp == 0;
  }

  [[nodiscard]]
  constexpr bool is_neq(partial_ordering __cmp) noexcept {
    return __cmp != 0;
  }

  [[nodiscard]]
  constexpr bool is_lt(partial_ordering __cmp) noexcept {
    return __cmp < 0;
  }

  [[nodiscard]]
  constexpr bool is_lteq(partial_ordering __cmp) noexcept {
    return __cmp <= 0;
  }

  [[nodiscard]]
  constexpr bool is_gt(partial_ordering __cmp) noexcept {
    return __cmp > 0;
  }

  [[nodiscard]]
  constexpr bool is_gteq(partial_ordering __cmp) noexcept {
    return __cmp >= 0;
  }

  namespace __detail {
    template <typename _Tp>
    inline constexpr unsigned __cmp_cat_id = 1;
    template <>
    inline constexpr unsigned __cmp_cat_id<partial_ordering> = 2;
    template <>
    inline constexpr unsigned __cmp_cat_id<weak_ordering> = 4;
    template <>
    inline constexpr unsigned __cmp_cat_id<strong_ordering> = 8;

    template <typename... _Ts>
    constexpr auto __common_cmp_cat() {
      constexpr unsigned __cats = (__cmp_cat_id<_Ts> | ...);
      // If any Ti is not a comparison category type, U is void.
      if constexpr (__cats & 1)
        return;
      // Otherwise, if at least one Ti is std::partial_ordering,
      // U is std::partial_ordering.
      else if constexpr (bool(__cats & __cmp_cat_id<partial_ordering>))
        return partial_ordering::equivalent;
      // Otherwise, if at least one Ti is std::weak_ordering,
      // U is std::weak_ordering.
      else if constexpr (bool(__cats & __cmp_cat_id<weak_ordering>))
        return weak_ordering::equivalent;
      // Otherwise, U is std::strong_ordering.
      else
        return strong_ordering::equivalent;
    }
  } // namespace __detail

  // [cmp.common], common comparison category type
  template <typename... _Ts>
  struct common_comparison_category {
    using type = decltype(__detail::__common_cmp_cat<_Ts...>());
  };

  // Partial specializations for one and zero argument cases.

  template <typename _Tp>
  struct common_comparison_category<_Tp> {
    using type = void;
  };

  template <>
  struct common_comparison_category<partial_ordering> {
    using type = partial_ordering;
  };

  template <>
  struct common_comparison_category<weak_ordering> {
    using type = weak_ordering;
  };

  template <>
  struct common_comparison_category<strong_ordering> {
    using type = strong_ordering;
  };

  template <>
  struct common_comparison_category<> {
    using type = strong_ordering;
  };

  template <typename... _Ts>
  using common_comparison_category_t = typename common_comparison_category<_Ts...>::type;

#if __cpp_lib_concepts
  namespace __detail {
    template <typename _Tp, typename _Cat>
    concept __compares_as = same_as<common_comparison_category_t<_Tp, _Cat>, _Cat>;
  } // namespace __detail

  // [cmp.concept], concept three_way_comparable
  template <typename _Tp, typename _Cat = partial_ordering>
  concept three_way_comparable =
    __detail::__weakly_eq_cmp_with<_Tp, _Tp> && __detail::__partially_ordered_with<_Tp, _Tp> &&
    requires(const remove_reference_t<_Tp>& __a, const remove_reference_t<_Tp>& __b) {
      { __a <=> __b } -> __detail::__compares_as<_Cat>;
    };

  template <typename _Tp, typename _Up, typename _Cat = partial_ordering>
  concept three_way_comparable_with =
    three_way_comparable<_Tp, _Cat> && three_way_comparable<_Up, _Cat> &&
    common_reference_with<const remove_reference_t<_Tp>&, const remove_reference_t<_Up>&> &&
    three_way_comparable<
      common_reference_t<const remove_reference_t<_Tp>&, const remove_reference_t<_Up>&>, _Cat> &&
    __detail::__weakly_eq_cmp_with<_Tp, _Up> && __detail::__partially_ordered_with<_Tp, _Up> &&
    requires(const remove_reference_t<_Tp>& __t, const remove_reference_t<_Up>& __u) {
      { __t <=> __u } -> __detail::__compares_as<_Cat>;
      { __u <=> __t } -> __detail::__compares_as<_Cat>;
    };

  namespace __detail {
    template <typename _Tp, typename _Up>
    using __cmp3way_res_t = decltype(std::declval<_Tp>() <=> std::declval<_Up>());

    // Implementation of std::compare_three_way_result.
    // It is undefined for a program to add specializations of
    // std::compare_three_way_result, so the std::compare_three_way_result_t
    // alias ignores std::compare_three_way_result and uses
    // __detail::__cmp3way_res_impl directly instead.
    template <typename _Tp, typename _Up>
    struct __cmp3way_res_impl {};

    template <typename _Tp, typename _Up>
      requires requires { typename __cmp3way_res_t<__cref<_Tp>, __cref<_Up>>; }
    struct __cmp3way_res_impl<_Tp, _Up> {
      using type = __cmp3way_res_t<__cref<_Tp>, __cref<_Up>>;
    };
  } // namespace __detail

  /// [cmp.result], result of three-way comparison
  template <typename _Tp, typename _Up = _Tp>
  struct compare_three_way_result : __detail::__cmp3way_res_impl<_Tp, _Up> {};

  /// [cmp.result], result of three-way comparison
  template <typename _Tp, typename _Up = _Tp>
  using compare_three_way_result_t = typename __detail::__cmp3way_res_impl<_Tp, _Up>::type;

  namespace __detail {
    // BUILTIN-PTR-THREE-WAY(T, U)
    // This determines whether t <=> u results in a call to a built-in
    // operator<=> comparing pointers. It doesn't work for function pointers
    // (PR 93628).
    template <typename _Tp, typename _Up>
    concept __3way_builtin_ptr_cmp =
      requires(_Tp&& __t, _Up&& __u) { static_cast<_Tp &&>(__t) <=> static_cast<_Up &&>(__u); } &&
      convertible_to<_Tp, const volatile void*> && convertible_to<_Up, const volatile void*> &&
      !requires(_Tp&& __t, _Up&& __u) {
        operator<=>(static_cast<_Tp &&>(__t), static_cast<_Up &&>(__u));
      } && !requires(_Tp&& __t, _Up&& __u) {
        static_cast<_Tp &&>(__t).operator<=>(static_cast<_Up &&>(__u));
      };
  } // namespace __detail

  // _GLIBCXX_RESOLVE_LIB_DEFECTS
  // 3530 BUILTIN-PTR-MEOW should not opt the type out of syntactic checks

  // [cmp.object], typename compare_three_way
  struct compare_three_way {
    template <typename _Tp, typename _Up>
      requires three_way_comparable_with<_Tp, _Up>
    constexpr auto operator() [[nodiscard]] (_Tp&& __t, _Up&& __u) const
      noexcept(noexcept(std::declval<_Tp>() <=> std::declval<_Up>())) {
      if constexpr (__detail::__3way_builtin_ptr_cmp<_Tp, _Up>) {
        auto __pt = static_cast<const volatile void*>(__t);
        auto __pu = static_cast<const volatile void*>(__u);
        if (std::__is_constant_evaluated())
          return __pt <=> __pu;
        auto __it = reinterpret_cast<__UINTPTR_TYPE__>(__pt);
        auto __iu = reinterpret_cast<__UINTPTR_TYPE__>(__pu);
        return __it <=> __iu;
      } else
        return static_cast<_Tp&&>(__t) <=> static_cast<_Up&&>(__u);
    }

    using is_transparent = void;
  };

  namespace __cmp_cust {
    template <floating_point _Tp>
    constexpr weak_ordering __fp_weak_ordering(_Tp __e, _Tp __f) {
      // Returns an integer with the same sign as the argument, and magnitude
      // indicating the classification: zero=1 subnorm=2 norm=3 inf=4 nan=5
      auto __cat = [](_Tp __fp) -> int {
        const int __sign = __builtin_signbit(__fp) ? -1 : 1;
        if (__builtin_isnormal(__fp))
          return (__fp == 0 ? 1 : 3) * __sign;
        if (__builtin_isnan(__fp))
          return 5 * __sign;
        if (int __inf = __builtin_isinf_sign(__fp))
          return 4 * __inf;
        return 2 * __sign;
      };

      auto __po = __e <=> __f;
      if (is_lt(__po))
        return weak_ordering::less;
      else if (is_gt(__po))
        return weak_ordering::greater;
      else if (__po == partial_ordering::equivalent)
        return weak_ordering::equivalent;
      else // unordered, at least one argument is NaN
      {
        // return -1 for negative nan, +1 for positive nan, 0 otherwise.
        auto __isnan_sign = [](_Tp __fp) -> int {
          return __builtin_isnan(__fp) ? __builtin_signbit(__fp) ? -1 : 1 : 0;
        };
        auto __ord = __isnan_sign(__e) <=> __isnan_sign(__f);
        if (is_eq(__ord))
          return weak_ordering::equivalent;
        else if (is_lt(__ord))
          return weak_ordering::less;
        else
          return weak_ordering::greater;
      }
    }

    template <typename _Tp, typename _Up>
    concept __adl_strong = requires(_Tp&& __t, _Up&& __u) {
      strong_ordering(strong_order(static_cast<_Tp &&>(__t), static_cast<_Up &&>(__u)));
    };

    template <typename _Tp, typename _Up>
    concept __adl_weak = requires(_Tp&& __t, _Up&& __u) {
      weak_ordering(weak_order(static_cast<_Tp &&>(__t), static_cast<_Up &&>(__u)));
    };

    template <typename _Tp, typename _Up>
    concept __adl_partial = requires(_Tp&& __t, _Up&& __u) {
      partial_ordering(partial_order(static_cast<_Tp &&>(__t), static_cast<_Up &&>(__u)));
    };

    template <typename _Ord, typename _Tp, typename _Up>
    concept __cmp3way = requires(_Tp&& __t, _Up&& __u, compare_three_way __c) {
      _Ord(__c(static_cast<_Tp &&>(__t), static_cast<_Up &&>(__u)));
    };

    template <typename _Tp, typename _Up>
    concept __strongly_ordered =
      __adl_strong<_Tp, _Up> || floating_point<remove_reference_t<_Tp>> ||
      __cmp3way<strong_ordering, _Tp, _Up>;

    template <typename _Tp, typename _Up>
    concept __decayed_same_as = same_as<decay_t<_Tp>, decay_t<_Up>>;

    class _Strong_order {
      template <typename _Tp, typename _Up>
      static constexpr bool _S_noexcept() {
        if constexpr (floating_point<decay_t<_Tp>>)
          return true;
        else if constexpr (__adl_strong<_Tp, _Up>)
          return noexcept(strong_ordering(strong_order(std::declval<_Tp>(), std::declval<_Up>())));
        else if constexpr (__cmp3way<strong_ordering, _Tp, _Up>)
          return noexcept(compare_three_way()(std::declval<_Tp>(), std::declval<_Up>()));
      }

      friend class _Weak_order;
      friend class _Strong_fallback;

      // Names for the supported floating-point representations.
      enum class _Fp_fmt {
        _Binary16,
        _Binary32,
        _Binary64,
        _Binary128,  // IEEE
        _X86_80bit,  // x86 80-bit extended precision
        _M68k_80bit, // m68k 80-bit extended precision
        _Dbldbl,     // IBM 128-bit double-double
        _Bfloat16,   // std::bfloat16_t
      };

#ifndef __cpp_using_enum
      // XXX Remove these once 'using enum' support is ubiquitous.
      static constexpr _Fp_fmt _Binary16   = _Fp_fmt::_Binary16;
      static constexpr _Fp_fmt _Binary32   = _Fp_fmt::_Binary32;
      static constexpr _Fp_fmt _Binary64   = _Fp_fmt::_Binary64;
      static constexpr _Fp_fmt _Binary128  = _Fp_fmt::_Binary128;
      static constexpr _Fp_fmt _X86_80bit  = _Fp_fmt::_X86_80bit;
      static constexpr _Fp_fmt _M68k_80bit = _Fp_fmt::_M68k_80bit;
      static constexpr _Fp_fmt _Dbldbl     = _Fp_fmt::_Dbldbl;
      static constexpr _Fp_fmt _Bfloat16   = _Fp_fmt::_Bfloat16;
#endif

      // Identify the format used by a floating-point type.
      template <typename _Tp>
      static consteval _Fp_fmt _S_fp_fmt() noexcept {
#ifdef __cpp_using_enum
        using enum _Fp_fmt;
#endif

        // Identify these formats first, then assume anything else is IEEE.
        // N.B. ARM __fp16 alternative format can be handled as binary16.

#ifdef __LONG_DOUBLE_IBM128__
        if constexpr (__is_same(_Tp, long double))
          return _Dbldbl;
#elif defined __LONG_DOUBLE_IEEE128__ && defined __SIZEOF_IBM128__
        if constexpr (__is_same(_Tp, __ibm128))
          return _Dbldbl;
#endif

#if __LDBL_MANT_DIG__ == 64
        if constexpr (__is_same(_Tp, long double))
          return __LDBL_MIN_EXP__ == -16381 ? _X86_80bit : _M68k_80bit;
#endif
#ifdef __SIZEOF_FLOAT80__
        if constexpr (__is_same(_Tp, __float80))
          return _X86_80bit;
#endif
#ifdef __STDCPP_BFLOAT16_T__
        if constexpr (__is_same(_Tp, decltype(0.0bf16)))
          return _Bfloat16;
#endif

        constexpr int __width = sizeof(_Tp) * __CHAR_BIT__;

        if constexpr (__width == 16) // IEEE binary16 (or ARM fp16).
          return _Binary16;
        else if constexpr (__width == 32) // IEEE binary32
          return _Binary32;
        else if constexpr (__width == 64) // IEEE binary64
          return _Binary64;
        else if constexpr (__width == 128) // IEEE binary128
          return _Binary128;
      }

      // So we don't need to include <stdint.h> and pollute the namespace.
      using int64_t  = __INT64_TYPE__;
      using int32_t  = __INT32_TYPE__;
      using int16_t  = __INT16_TYPE__;
      using uint64_t = __UINT64_TYPE__;
      using uint16_t = __UINT16_TYPE__;

      // Used to unpack floating-point types that do not fit into an integer.
      template <typename _Tp>
      struct _Int {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        uint64_t _M_lo;
        _Tp      _M_hi;
#else
        _Tp      _M_hi;
        uint64_t _M_lo;
#endif

        constexpr explicit _Int(_Tp __hi, uint64_t __lo) noexcept : _M_hi(__hi) { _M_lo = __lo; }

        constexpr explicit _Int(uint64_t __lo) noexcept : _M_hi(0) { _M_lo = __lo; }

        constexpr bool operator==(const _Int&) const = default;

#if defined __hppa__ || (defined __mips__ && !defined __mips_nan2008)
        consteval _Int operator<<(int __n) const noexcept {
          // XXX this assumes n >= 64, which is true for the use below.
          return _Int(static_cast<_Tp>(_M_lo << (__n - 64)), 0);
        }
#endif

        constexpr _Int& operator^=(const _Int& __rhs) noexcept {
          _M_hi ^= __rhs._M_hi;
          _M_lo ^= __rhs._M_lo;
          return *this;
        }

        constexpr strong_ordering operator<=>(const _Int& __rhs) const noexcept {
          strong_ordering __cmp = _M_hi <=> __rhs._M_hi;
          if (__cmp != strong_ordering::equal)
            return __cmp;
          return _M_lo <=> __rhs._M_lo;
        }
      };

      template <typename _Tp>
      static constexpr _Tp _S_compl(_Tp __t) noexcept {
        constexpr int __width = sizeof(_Tp) * __CHAR_BIT__;
        // Sign extend to get all ones or all zeros.
        make_unsigned_t<_Tp> __sign = __t >> (__width - 1);
        // If the sign bit was set, this flips all bits below it.
        // This converts ones' complement to two's complement.
        return __t ^ (__sign >> 1);
      }

      // As above but works on both parts of _Int<T>.
      template <typename _Tp>
      static constexpr _Int<_Tp> _S_compl(_Int<_Tp> __t) noexcept {
        constexpr int        __width = sizeof(_Tp) * __CHAR_BIT__;
        make_unsigned_t<_Tp> __sign  = __t._M_hi >> (__width - 1);
        __t._M_hi ^= (__sign >> 1);
        uint64_t __sign64 = (_Tp)__sign;
        __t._M_lo ^= __sign64;
        return __t;
      }

      // Bit-cast a floating-point value to an unsigned integer.
      template <typename _Tp>
      constexpr static auto _S_fp_bits(_Tp __val) noexcept {
        if constexpr (sizeof(_Tp) == sizeof(int64_t))
          return __builtin_bit_cast(int64_t, __val);
        else if constexpr (sizeof(_Tp) == sizeof(int32_t))
          return __builtin_bit_cast(int32_t, __val);
        else if constexpr (sizeof(_Tp) == sizeof(int16_t))
          return __builtin_bit_cast(int16_t, __val);
        else {
#ifdef __cpp_using_enum
          using enum _Fp_fmt;
#endif
          constexpr auto __fmt = _S_fp_fmt<_Tp>();
          if constexpr (__fmt == _X86_80bit || __fmt == _M68k_80bit) {
            if constexpr (sizeof(_Tp) == 3 * sizeof(int32_t)) {
              auto __ival = __builtin_bit_cast(_Int<int32_t>, __val);
              return _Int<int16_t>(__ival._M_hi, __ival._M_lo);
            } else {
              auto __ival = __builtin_bit_cast(_Int<int64_t>, __val);
              return _Int<int16_t>(__ival._M_hi, __ival._M_lo);
            }
          } else if constexpr (sizeof(_Tp) == 2 * sizeof(int64_t)) {
#if __SIZEOF_INT128__
            return __builtin_bit_cast(__int128, __val);
#else
            return __builtin_bit_cast(_Int<int64_t>, __val);
#endif
          } else
            static_assert(sizeof(_Tp) == sizeof(int64_t), "unsupported floating-point type");
        }
      }

      template <typename _Tp>
      static constexpr strong_ordering _S_fp_cmp(_Tp __x, _Tp __y) noexcept {
#ifdef __vax__
        if (__builtin_isnan(__x) || __builtin_isnan(__y)) {
          int __ix = (bool)__builtin_isnan(__x);
          int __iy = (bool)__builtin_isnan(__y);
          __ix *= __builtin_signbit(__x) ? -1 : 1;
          __iy *= __builtin_signbit(__y) ? -1 : 1;
          return __ix <=> __iy;
        } else
          return __builtin_bit_cast(strong_ordering, __x <=> __y);
#endif

        auto __ix = _S_fp_bits(__x);
        auto __iy = _S_fp_bits(__y);

        if (__ix == __iy)
          return strong_ordering::equal; // All bits are equal, we're done.

#ifdef __cpp_using_enum
        using enum _Fp_fmt;
#endif
        constexpr auto __fmt = _S_fp_fmt<_Tp>();

        if constexpr (__fmt == _Dbldbl) // double-double
        {
          // Unpack the double-double into two parts.
          // We never inspect the low double as a double, cast to integer.
          struct _Unpacked {
            double  _M_hi;
            int64_t _M_lo;
          };
          auto __x2 = __builtin_bit_cast(_Unpacked, __x);
          auto __y2 = __builtin_bit_cast(_Unpacked, __y);

          // Compare the high doubles first and use result if unequal.
          auto __cmp = _S_fp_cmp(__x2._M_hi, __y2._M_hi);
          if (__cmp != strong_ordering::equal)
            return __cmp;

          // For NaN the low double is unused, so if the high doubles
          // are the same NaN, we don't need to compare the low doubles.
          if (__builtin_isnan(__x2._M_hi))
            return strong_ordering::equal;
          // Similarly, if the low doubles are +zero or -zero (which is
          // true for all infinities and some other values), we're done.
          if (((__x2._M_lo | __y2._M_lo) & 0x7fffffffffffffffULL) == 0)
            return strong_ordering::equal;

          // Otherwise, compare the low parts.
          return _S_compl(__x2._M_lo) <=> _S_compl(__y2._M_lo);
        } else {
          if constexpr (__fmt == _M68k_80bit) {
            // For m68k the MSB of the significand is ignored for the
            // greatest exponent, so either 0 or 1 is valid there.
            // Set it before comparing, so that we never have 0 there.
            constexpr uint16_t __maxexp = 0x7fff;
            if ((__ix._M_hi & __maxexp) == __maxexp)
              __ix._M_lo |= 1ull << 63;
            if ((__iy._M_hi & __maxexp) == __maxexp)
              __iy._M_lo |= 1ull << 63;
          } else {
#if defined __hppa__ || (defined __mips__ && !defined __mips_nan2008)
            // IEEE 754-1985 allowed the meaning of the quiet/signaling
            // bit to be reversed. Flip that to give desired ordering.
            if (__builtin_isnan(__x) && __builtin_isnan(__y)) {
              using _Int = decltype(__ix);

              constexpr int  __nantype = __fmt == _Binary32    ? 22
                                         : __fmt == _Binary64  ? 51
                                         : __fmt == _Binary128 ? 111
                                                               : -1;
              constexpr _Int __bit     = _Int(1) << __nantype;
              __ix ^= __bit;
              __iy ^= __bit;
            }
#endif
          }
          return _S_compl(__ix) <=> _S_compl(__iy);
        }
      }

    public:
      template <typename _Tp, __decayed_same_as<_Tp> _Up>
        requires __strongly_ordered<_Tp, _Up>
      constexpr strong_ordering operator() [[nodiscard]] (_Tp&& __e, _Up&& __f) const
        noexcept(_S_noexcept<_Tp, _Up>()) {
        if constexpr (floating_point<decay_t<_Tp>>)
          return _S_fp_cmp(__e, __f);
        else if constexpr (__adl_strong<_Tp, _Up>)
          return strong_ordering(strong_order(static_cast<_Tp&&>(__e), static_cast<_Up&&>(__f)));
        else if constexpr (__cmp3way<strong_ordering, _Tp, _Up>)
          return compare_three_way()(static_cast<_Tp&&>(__e), static_cast<_Up&&>(__f));
      }
    };

    template <typename _Tp, typename _Up>
    concept __weakly_ordered = floating_point<remove_reference_t<_Tp>> || __adl_weak<_Tp, _Up> ||
                               __cmp3way<weak_ordering, _Tp, _Up> || __strongly_ordered<_Tp, _Up>;

    class _Weak_order {
      template <typename _Tp, typename _Up>
      static constexpr bool _S_noexcept() {
        if constexpr (floating_point<decay_t<_Tp>>)
          return true;
        else if constexpr (__adl_weak<_Tp, _Up>)
          return noexcept(weak_ordering(weak_order(std::declval<_Tp>(), std::declval<_Up>())));
        else if constexpr (__cmp3way<weak_ordering, _Tp, _Up>)
          return noexcept(compare_three_way()(std::declval<_Tp>(), std::declval<_Up>()));
        else if constexpr (__strongly_ordered<_Tp, _Up>)
          return _Strong_order::_S_noexcept<_Tp, _Up>();
      }

      friend class _Partial_order;
      friend class _Weak_fallback;

    public:
      template <typename _Tp, __decayed_same_as<_Tp> _Up>
        requires __weakly_ordered<_Tp, _Up>
      constexpr weak_ordering operator() [[nodiscard]] (_Tp&& __e, _Up&& __f) const
        noexcept(_S_noexcept<_Tp, _Up>()) {
        if constexpr (floating_point<decay_t<_Tp>>)
          return __cmp_cust::__fp_weak_ordering(__e, __f);
        else if constexpr (__adl_weak<_Tp, _Up>)
          return weak_ordering(weak_order(static_cast<_Tp&&>(__e), static_cast<_Up&&>(__f)));
        else if constexpr (__cmp3way<weak_ordering, _Tp, _Up>)
          return compare_three_way()(static_cast<_Tp&&>(__e), static_cast<_Up&&>(__f));
        else if constexpr (__strongly_ordered<_Tp, _Up>)
          return _Strong_order {}(static_cast<_Tp&&>(__e), static_cast<_Up&&>(__f));
      }
    };

    template <typename _Tp, typename _Up>
    concept __partially_ordered =
      __adl_partial<_Tp, _Up> || __cmp3way<partial_ordering, _Tp, _Up> ||
      __weakly_ordered<_Tp, _Up>;

    class _Partial_order {
      template <typename _Tp, typename _Up>
      static constexpr bool _S_noexcept() {
        if constexpr (__adl_partial<_Tp, _Up>)
          return noexcept(
            partial_ordering(partial_order(std::declval<_Tp>(), std::declval<_Up>())));
        else if constexpr (__cmp3way<partial_ordering, _Tp, _Up>)
          return noexcept(compare_three_way()(std::declval<_Tp>(), std::declval<_Up>()));
        else if constexpr (__weakly_ordered<_Tp, _Up>)
          return _Weak_order::_S_noexcept<_Tp, _Up>();
      }

      friend class _Partial_fallback;

    public:
      template <typename _Tp, __decayed_same_as<_Tp> _Up>
        requires __partially_ordered<_Tp, _Up>
      constexpr partial_ordering operator() [[nodiscard]] (_Tp&& __e, _Up&& __f) const
        noexcept(_S_noexcept<_Tp, _Up>()) {
        if constexpr (__adl_partial<_Tp, _Up>)
          return partial_ordering(partial_order(static_cast<_Tp&&>(__e), static_cast<_Up&&>(__f)));
        else if constexpr (__cmp3way<partial_ordering, _Tp, _Up>)
          return compare_three_way()(static_cast<_Tp&&>(__e), static_cast<_Up&&>(__f));
        else if constexpr (__weakly_ordered<_Tp, _Up>)
          return _Weak_order {}(static_cast<_Tp&&>(__e), static_cast<_Up&&>(__f));
      }
    };

    template <typename _Tp, typename _Up>
    concept __op_eq_lt = requires(_Tp&& __t, _Up&& __u) {
      { static_cast<_Tp &&>(__t) == static_cast<_Up &&>(__u) } -> convertible_to<bool>;
      { static_cast<_Tp &&>(__t) < static_cast<_Up &&>(__u) } -> convertible_to<bool>;
    };

    class _Strong_fallback {
      template <typename _Tp, typename _Up>
      static constexpr bool _S_noexcept() {
        if constexpr (__strongly_ordered<_Tp, _Up>)
          return _Strong_order::_S_noexcept<_Tp, _Up>();
        else
          return noexcept(bool(std::declval<_Tp>() == std::declval<_Up>())) &&
                 noexcept(bool(std::declval<_Tp>() < std::declval<_Up>()));
      }

    public:
      template <typename _Tp, __decayed_same_as<_Tp> _Up>
        requires __strongly_ordered<_Tp, _Up> || __op_eq_lt<_Tp, _Up>
      constexpr strong_ordering operator() [[nodiscard]] (_Tp&& __e, _Up&& __f) const
        noexcept(_S_noexcept<_Tp, _Up>()) {
        if constexpr (__strongly_ordered<_Tp, _Up>)
          return _Strong_order {}(static_cast<_Tp&&>(__e), static_cast<_Up&&>(__f));
        else // __op_eq_lt<_Tp, _Up>
          return static_cast<_Tp&&>(__e) == static_cast<_Up&&>(__f)  ? strong_ordering::equal
                 : static_cast<_Tp&&>(__e) < static_cast<_Up&&>(__f) ? strong_ordering::less
                                                                     : strong_ordering::greater;
      }
    };

    class _Weak_fallback {
      template <typename _Tp, typename _Up>
      static constexpr bool _S_noexcept() {
        if constexpr (__weakly_ordered<_Tp, _Up>)
          return _Weak_order::_S_noexcept<_Tp, _Up>();
        else
          return noexcept(bool(std::declval<_Tp>() == std::declval<_Up>())) &&
                 noexcept(bool(std::declval<_Tp>() < std::declval<_Up>()));
      }

    public:
      template <typename _Tp, __decayed_same_as<_Tp> _Up>
        requires __weakly_ordered<_Tp, _Up> || __op_eq_lt<_Tp, _Up>
      constexpr weak_ordering operator() [[nodiscard]] (_Tp&& __e, _Up&& __f) const
        noexcept(_S_noexcept<_Tp, _Up>()) {
        if constexpr (__weakly_ordered<_Tp, _Up>)
          return _Weak_order {}(static_cast<_Tp&&>(__e), static_cast<_Up&&>(__f));
        else // __op_eq_lt<_Tp, _Up>
          return static_cast<_Tp&&>(__e) == static_cast<_Up&&>(__f)  ? weak_ordering::equivalent
                 : static_cast<_Tp&&>(__e) < static_cast<_Up&&>(__f) ? weak_ordering::less
                                                                     : weak_ordering::greater;
      }
    };

    // _GLIBCXX_RESOLVE_LIB_DEFECTS
    // 3465. compare_partial_order_fallback requires F < E
    template <typename _Tp, typename _Up>
    concept __op_eq_lt_lt = __op_eq_lt<_Tp, _Up> && requires(_Tp&& __t, _Up&& __u) {
      { static_cast<_Up &&>(__u) < static_cast<_Tp &&>(__t) } -> convertible_to<bool>;
    };

    class _Partial_fallback {
      template <typename _Tp, typename _Up>
      static constexpr bool _S_noexcept() {
        if constexpr (__partially_ordered<_Tp, _Up>)
          return _Partial_order::_S_noexcept<_Tp, _Up>();
        else
          return noexcept(bool(std::declval<_Tp>() == std::declval<_Up>())) &&
                 noexcept(bool(std::declval<_Tp>() < std::declval<_Up>()));
      }

    public:
      template <typename _Tp, __decayed_same_as<_Tp> _Up>
        requires __partially_ordered<_Tp, _Up> || __op_eq_lt_lt<_Tp, _Up>
      constexpr partial_ordering operator() [[nodiscard]] (_Tp&& __e, _Up&& __f) const
        noexcept(_S_noexcept<_Tp, _Up>()) {
        if constexpr (__partially_ordered<_Tp, _Up>)
          return _Partial_order {}(static_cast<_Tp&&>(__e), static_cast<_Up&&>(__f));
        else // __op_eq_lt_lt<_Tp, _Up>
          return static_cast<_Tp&&>(__e) == static_cast<_Up&&>(__f)  ? partial_ordering::equivalent
                 : static_cast<_Tp&&>(__e) < static_cast<_Up&&>(__f) ? partial_ordering::less
                 : static_cast<_Up&&>(__f) < static_cast<_Tp&&>(__e) ? partial_ordering::greater
                                                                     : partial_ordering::unordered;
      }
    };
  } // namespace __cmp_cust

  // [cmp.alg], comparison algorithms
  inline namespace __cmp_alg {
    inline constexpr __cmp_cust::_Strong_order strong_order {};

    inline constexpr __cmp_cust::_Weak_order weak_order {};

    inline constexpr __cmp_cust::_Partial_order partial_order {};

    inline constexpr __cmp_cust::_Strong_fallback compare_strong_order_fallback {};

    inline constexpr __cmp_cust::_Weak_fallback compare_weak_order_fallback {};

    inline constexpr __cmp_cust::_Partial_fallback compare_partial_order_fallback {};
  } // namespace __cmp_alg

  namespace __detail {
    // [expos.only.func] synth-three-way
    inline constexpr struct _Synth3way {
      template <typename _Tp, typename _Up>
      static constexpr bool _S_noexcept(const _Tp* __t = nullptr, const _Up* __u = nullptr) {
        if constexpr (three_way_comparable_with<_Tp, _Up>)
          return noexcept(*__t <=> *__u);
        else
          return noexcept(*__t < *__u) && noexcept(*__u < *__t);
      }

      template <typename _Tp, typename _Up>
      [[nodiscard]]
      constexpr auto operator()(const _Tp& __t, const _Up& __u) const
        noexcept(_S_noexcept<_Tp, _Up>())
        requires requires {
          { __t < __u } -> __boolean_testable;
          { __u < __t } -> __boolean_testable;
        }
      {
        if constexpr (three_way_comparable_with<_Tp, _Up>)
          return __t <=> __u;
        else {
          if (__t < __u)
            return weak_ordering::less;
          else if (__u < __t)
            return weak_ordering::greater;
          else
            return weak_ordering::equivalent;
        }
      }
    } __synth3way = {};

    // [expos.only.func] synth-three-way-result
    template <typename _Tp, typename _Up = _Tp>
    using __synth3way_t =
      decltype(__detail::__synth3way(std::declval<_Tp&>(), std::declval<_Up&>()));
  } // namespace __detail
#endif // concepts
} // namespace std _GLIBCXX_VISIBILITY(default)

#endif // C++20

#endif // _COMPARE
