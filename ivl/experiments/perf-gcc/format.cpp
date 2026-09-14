#include <span>

#include <bits/requires_hosted.h>

#define __glibcxx_want_format
#define __glibcxx_want_format_ranges
#define __glibcxx_want_format_uchar
#define __glibcxx_want_constexpr_exceptions
#define __glibcxx_want_constexpr_format
#include <bits/version.h>

#ifdef __cpp_lib_format

#include <bits/formatfwd.h>
#include <bits/monostate.h>
#include <bits/ranges_algobase.h>
#include <bits/ranges_base.h>
#include <bits/ranges_util.h>
#include <bits/stl_iterator.h>
#include <bits/stl_pair.h>
#include <bits/unicode.h>
#include <bits/utility.h>
#include <ext/numeric_traits.h>
#include <array>
#include <charconv>
#include <concepts>
#include <limits>
#include <span>
#include <string>
#include <string_view>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

namespace std _GLIBCXX_VISIBILITY(default) {
_GLIBCXX_BEGIN_NAMESPACE_VERSION

template<typename _CharT, typename... _Args>
struct basic_format_string;

namespace __format {

  template<typename _CharT>
  consteval auto _Widen(const char* __narrow, const wchar_t* __wide) {
    if constexpr (is_same_v<_CharT, wchar_t>) return __wide;
    else return __narrow;
  }
#define _GLIBCXX_WIDEN_(C, S) ::std::__format::_Widen<C>(S, L##S)
#define _GLIBCXX_WIDEN(S) _GLIBCXX_WIDEN_(_CharT, S)

  template<typename _CharT>
  constexpr size_t __stackbuf_size = 32 * sizeof(void*) / sizeof(_CharT);

  template<typename _CharT>
  class _Sink;
  template<typename _CharT>
  class _Fixedbuf_sink;
  template<typename _Out, typename _CharT>
  class _Padding_sink;
  template<typename _Out, typename _CharT>
  class _Escaping_sink;

  template<typename _CharT>
  class _Sink_iter;

  template<typename _CharT>
  class _Drop_iter;

  template<typename _CharT>
  struct _Iter_for {
    using type = _Drop_iter<_CharT>;
  };

  template<typename _CharT>
  using __format_context = basic_format_context<_Sink_iter<_CharT>, _CharT>;

  template<typename _CharT>
  struct _Dynamic_format_string {
    [[__gnu__::__always_inline__]]
    constexpr _Dynamic_format_string(basic_string_view<_CharT> __s) noexcept
        : _M_str(__s) {}

    _Dynamic_format_string(const _Dynamic_format_string&) = delete;
    void operator=(const _Dynamic_format_string&) = delete;

  private:
    basic_string_view<_CharT> _M_str;

    template<typename, typename...>
    friend struct std::basic_format_string;
  };

} // namespace __format

using format_context = __format::__format_context<char>;

template<typename _Context>
class basic_format_args;
using format_args = basic_format_args<format_context>;

template<typename _Context>
class basic_format_arg;

/** A compile-time checked format string for the specified argument types.
 *
 * @since C++23 but available as an extension in C++20.
 */
template<typename _CharT, typename... _Args>
struct basic_format_string {
  template<typename _Tp>
    requires convertible_to<const _Tp&, basic_string_view<_CharT>>
  consteval basic_format_string(const _Tp& __s) noexcept;

  [[__gnu__::__always_inline__]]
  constexpr basic_format_string(__format::_Dynamic_format_string<_CharT> __s) noexcept
      : _M_str(__s._M_str) {}

  [[__gnu__::__always_inline__]]
  constexpr basic_string_view<_CharT> get() const noexcept {
    return _M_str;
  }

private:
  basic_string_view<_CharT> _M_str;
};

template<typename... _Args>
using format_string = basic_format_string<char, type_identity_t<_Args>...>;

#if __cpp_lib_format >= 202603L
[[__gnu__::__always_inline__]]
inline constexpr __format::_Dynamic_format_string<char> dynamic_format(string_view __fmt) noexcept {
  return __fmt;
}
#endif

template<typename _Tp, typename _CharT>
struct formatter {
  formatter() = delete;
  formatter(const formatter&) = delete;
  formatter& operator=(const formatter&) = delete;
};

class format_error : public runtime_error {
public:
  constexpr explicit format_error(const string& __what) : runtime_error(__what) {}

  constexpr explicit format_error(const char* __what) : runtime_error(__what) {}
};

[[noreturn]]
inline constexpr void __throw_format_error(const char* __what) {
  _GLIBCXX_THROW_OR_ABORT(format_error(__what));
}

namespace __format {

  [[noreturn]]
  inline constexpr void __unmatched_left_brace_in_format_string() {
    __throw_format_error("format error: unmatched '{' in format string");
  }

  [[noreturn]]
  inline constexpr void __unmatched_right_brace_in_format_string() {
    __throw_format_error("format error: unmatched '}' in format string");
  }

  [[noreturn]]
  inline constexpr void __conflicting_indexing_in_format_string() {
    __throw_format_error("format error: conflicting indexing style in format string");
  }

  [[noreturn]]
  inline constexpr void __invalid_arg_id_in_format_string() {
    __throw_format_error("format error: invalid arg-id in format string");
  }

  [[noreturn]]
  inline constexpr void __failed_to_parse_format_spec() {
    __throw_format_error("format error: failed to parse format-spec");
  }

  template<typename _CharT>
  class _Scanner;

} // namespace __format

template<typename _CharT>
class basic_format_parse_context;
using format_parse_context = basic_format_parse_context<char>;

template<typename _CharT>

class _GLIBCXX_NO_SPECIALIZATIONS basic_format_parse_context {
public:
  using char_type = _CharT;
  using const_iterator = typename basic_string_view<_CharT>::const_iterator;
  using iterator = const_iterator;

  constexpr explicit basic_format_parse_context(basic_string_view<_CharT> __fmt) noexcept
      : _M_begin(__fmt.begin()), _M_end(__fmt.end()) {}

  basic_format_parse_context(const basic_format_parse_context&) = delete;
  void operator=(const basic_format_parse_context&) = delete;

  constexpr const_iterator begin() const noexcept { return _M_begin; }
  constexpr const_iterator end() const noexcept { return _M_end; }

  constexpr void advance_to(const_iterator __it) noexcept { _M_begin = __it; }

  constexpr size_t next_arg_id() {
    if (_M_indexing == _Manual) __format::__conflicting_indexing_in_format_string();
    _M_indexing = _Auto;

    if (std::is_constant_evaluated())
      if (_M_next_arg_id == _M_num_args) __format::__invalid_arg_id_in_format_string();
    return _M_next_arg_id++;
  }

  constexpr void check_arg_id(size_t __id) {
    if (_M_indexing == _Auto) __format::__conflicting_indexing_in_format_string();
    _M_indexing = _Manual;

    if (std::is_constant_evaluated())
      if (__id >= _M_num_args) __format::__invalid_arg_id_in_format_string();
  }

#if __cpp_lib_format >= 202305L
  template<typename... _Ts>
  constexpr void check_dynamic_spec(size_t __id) noexcept {
    static_assert(
      __valid_types_for_check_dynamic_spec<_Ts...>(), "template arguments for check_dynamic_spec<Ts...>(id) "
                                                      "must be unique and must be one of the allowed types"
    );
    if consteval {
      __check_dynamic_spec<_Ts...>(__id);
    }
  }

  constexpr void check_dynamic_spec_integral(size_t __id) noexcept {
    if consteval {
      __check_dynamic_spec<int, unsigned, long long, unsigned long long>(__id);
    }
  }

  constexpr void check_dynamic_spec_string(size_t __id) noexcept {
    if consteval {
      __check_dynamic_spec<const _CharT*, basic_string_view<_CharT>>(__id);
    }
  }

private:
  template<typename _Tp, typename... _Ts>
  static constexpr bool __once = (is_same_v<_Tp, _Ts> + ...) == 1;

  template<typename... _Ts>
  consteval bool __valid_types_for_check_dynamic_spec() {
    if constexpr (sizeof...(_Ts) == 0) return false;
    else {
      unsigned __sum = __once<bool, _Ts...> + __once<char_type, _Ts...> + __once<int, _Ts...> +
                       __once<unsigned int, _Ts...> + __once<long long int, _Ts...> +
                       __once<unsigned long long int, _Ts...> + __once<float, _Ts...> + __once<double, _Ts...> +
                       __once<long double, _Ts...> + __once<const char_type*, _Ts...> +
                       __once<basic_string_view<char_type>, _Ts...> + __once<const void*, _Ts...>;
      return __sum == sizeof...(_Ts);
    }
  }

  template<typename... _Ts>
  consteval void __check_dynamic_spec(size_t __id) noexcept;

  static void __invalid_dynamic_spec(const char*);

  friend __format::_Scanner<_CharT>;
#endif

  constexpr explicit basic_format_parse_context(basic_string_view<_CharT> __fmt, size_t __num_args) noexcept
      : _M_begin(__fmt.begin()), _M_end(__fmt.end()), _M_num_args(__num_args) {}

private:
  iterator _M_begin;
  iterator _M_end;
  enum _Indexing { _Unknown, _Manual, _Auto };
  _Indexing _M_indexing = _Unknown;
  size_t _M_next_arg_id = 0;
  size_t _M_num_args = 0;
};

template<typename _Tp, template<typename...> class _Class>
constexpr bool __is_specialization_of = false;
template<template<typename...> class _Class, typename... _Args>
constexpr bool __is_specialization_of<_Class<_Args...>, _Class> = true;

namespace __format {

  template<typename _CharT>
  constexpr pair<unsigned short, const _CharT*> __parse_integer(const _CharT* __first, const _CharT* __last) {
    if (__first == __last) __builtin_unreachable();

    if constexpr (is_same_v<_CharT, char>) {
      const auto __start = __first;
      unsigned short __val = 0;

      if (__detail::__from_chars_alnum<true>(__first, __last, __val, 10) && __first != __start) [[likely]]
        return {__val, __first};
    } else {
      constexpr int __n = 32;
      char __buf[__n]{};
      for (int __i = 0; __i < __n && (__first + __i) != __last; ++__i) __buf[__i] = __first[__i];
      auto [__v, __ptr] = __format::__parse_integer(__buf, __buf + __n);
      if (__ptr) [[likely]]
        return {__v, __first + (__ptr - __buf)};
    }
    return {0, nullptr};
  }

  template<typename _CharT>
  constexpr pair<unsigned short, const _CharT*> __parse_arg_id(const _CharT* __first, const _CharT* __last) {
    if (__first == __last) __builtin_unreachable();

    if (*__first == '0') return {0, __first + 1};

    if ('1' <= *__first && *__first <= '9') {
      const unsigned short __id = *__first - '0';
      const auto __next = __first + 1;

      if (__next == __last || !('0' <= *__next && *__next <= '9')) return {__id, __next};
      else return __format::__parse_integer(__first, __last);
    }
    return {0, nullptr};
  }

  enum class _Pres_type : unsigned char {
    _Pres_none = 0,
    _Pres_s = 1,

    _Pres_c = 2,
    _Pres_x,
    _Pres_X,
    _Pres_d,
    _Pres_o,
    _Pres_b,
    _Pres_B,

    _Pres_g = 1,
    _Pres_G,
    _Pres_a,
    _Pres_A,
    _Pres_e,
    _Pres_E,
    _Pres_f,
    _Pres_F,
    _Pres_p,
    _Pres_P,
    _Pres_max = 0xf,
  };
  using enum _Pres_type;

  enum class _Sign : unsigned char {
    _Sign_default,
    _Sign_plus,
    _Sign_minus,
    _Sign_space,
  };
  using enum _Sign;

  enum _WidthPrec : unsigned char { _WP_none, _WP_value, _WP_from_arg };
  using enum _WidthPrec;

  template<typename _Context>
  constexpr size_t __int_from_arg(const basic_format_arg<_Context>& __arg);

  constexpr bool __is_digit(char __c) { return std::__detail::__from_chars_alnum_to_val(__c) < 10; }

  constexpr bool __is_xdigit(char __c) { return std::__detail::__from_chars_alnum_to_val(__c) < 16; }

  struct _SpecBase {};

  template<typename _CharT>
  struct _Spec : _SpecBase {
    unsigned short _M_width;
    unsigned short _M_prec;
    char32_t _M_fill = ' ';
    _Align _M_align : 2;
    _Sign _M_sign : 2;
    unsigned _M_alt : 1;
    unsigned _M_localized : 1;
    unsigned _M_zero_fill : 1;
    _WidthPrec _M_width_kind : 2;
    _WidthPrec _M_prec_kind : 2;
    unsigned _M_debug : 1;
    _Pres_type _M_type : 4;
    unsigned _M_reserved : 8;

    using iterator = typename basic_string_view<_CharT>::iterator;

    static constexpr _Align _S_align(_CharT __c) noexcept {
      switch (__c) {
      case '<':
        return _Align_left;
      case '>':
        return _Align_right;
      case '^':
        return _Align_centre;
      default:
        return _Align_default;
      }
    }

    constexpr iterator _M_parse_fill_and_align(iterator __first, iterator __last) noexcept {
      return _M_parse_fill_and_align(__first, __last, "{");
    }

    constexpr iterator _M_parse_fill_and_align(iterator __first, iterator __last, string_view __not_fill) noexcept {
      for (char __c : __not_fill)
        if (*__first == static_cast<_CharT>(__c)) return __first;

      using namespace __unicode;
      if constexpr (__literal_encoding_is_unicode<_CharT>()) {
        _Utf32_view<ranges::subrange<iterator>> __uv({__first, __last});
        if (!__uv.empty()) {
          auto __beg = __uv.begin();
          char32_t __c = *__beg++;
          if (__is_scalar_value(__c))
            if (auto __next = __beg.base(); __next != __last)
              if (_Align __align = _S_align(*__next); __align != _Align_default) {
                _M_fill = __c;
                _M_align = __align;
                return ++__next;
              }
        }
      } else if (__last - __first >= 2)
        if (_Align __align = _S_align(__first[1]); __align != _Align_default) {
          _M_fill = *__first;
          _M_align = __align;
          return __first + 2;
        }

      if (_Align __align = _S_align(__first[0]); __align != _Align_default) {
        _M_fill = ' ';
        _M_align = __align;
        return __first + 1;
      }
      return __first;
    }

    static constexpr _Sign _S_sign(_CharT __c) noexcept {
      switch (__c) {
      case '+':
        return _Sign_plus;
      case '-':
        return _Sign_minus;
      case ' ':
        return _Sign_space;
      default:
        return _Sign_default;
      }
    }

    constexpr iterator _M_parse_sign(iterator __first, iterator) noexcept {
      if (_Sign __sign = _S_sign(*__first); __sign != _Sign_default) {
        _M_sign = __sign;
        return __first + 1;
      }
      return __first;
    }

    constexpr iterator _M_parse_alternate_form(iterator __first, iterator) noexcept {
      if (*__first == '#') {
        _M_alt = true;
        ++__first;
      }
      return __first;
    }

    constexpr iterator _M_parse_zero_fill(iterator __first, iterator /* __last */) noexcept {
      if (*__first == '0') {
        _M_zero_fill = true;
        ++__first;
      }
      return __first;
    }

    static constexpr iterator _S_parse_width_or_precision(
      iterator __first, iterator __last, unsigned short& __val, bool& __arg_id, basic_format_parse_context<_CharT>& __pc
    ) {
      if (__format::__is_digit(*__first)) {
        auto [__v, __ptr] = __format::__parse_integer(__first, __last);
        if (!__ptr)
          __throw_format_error(
            "format error: invalid width or precision "
            "in format-spec"
          );
        __first = __ptr;
        __val = __v;
      } else if (*__first == '{') {
        __arg_id = true;
        ++__first;
        if (__first == __last) __format::__unmatched_left_brace_in_format_string();
        if (*__first == '}') __val = __pc.next_arg_id();
        else {
          auto [__v, __ptr] = __format::__parse_arg_id(__first, __last);
          if (__ptr == nullptr || __ptr == __last || *__ptr != '}') __format::__invalid_arg_id_in_format_string();
          __first = __ptr;
          __pc.check_arg_id(__v);
          __val = __v;
        }
#if __cpp_lib_format >= 202305L
        __pc.check_dynamic_spec_integral(__val);
#endif
        ++__first;
      }
      return __first;
    }

    constexpr iterator _M_parse_width(iterator __first, iterator __last, basic_format_parse_context<_CharT>& __pc) {
      bool __arg_id = false;
      if (*__first == '0')
        __throw_format_error(
          "format error: width must be non-zero in "
          "format string"
        );
      auto __next = _S_parse_width_or_precision(__first, __last, _M_width, __arg_id, __pc);
      if (__next != __first) _M_width_kind = __arg_id ? _WP_from_arg : _WP_value;
      return __next;
    }

    constexpr iterator _M_parse_precision(iterator __first, iterator __last, basic_format_parse_context<_CharT>& __pc) {
      if (__first[0] != '.') return __first;

      iterator __next = ++__first;
      bool __arg_id = false;
      if (__next != __last) __next = _S_parse_width_or_precision(__first, __last, _M_prec, __arg_id, __pc);
      if (__next == __first)
        __throw_format_error(
          "format error: missing precision after '.' in "
          "format string"
        );
      _M_prec_kind = __arg_id ? _WP_from_arg : _WP_value;
      return __next;
    }

    constexpr iterator _M_parse_locale(iterator __first, iterator /* __last */) noexcept {
      if (*__first == 'L') {
        _M_localized = true;
        ++__first;
      }
      return __first;
    }

    template<typename _Context>
    constexpr size_t _M_get_width(_Context& __ctx) const {
      size_t __width = 0;
      if (_M_width_kind == _WP_value) __width = _M_width;
      else if (_M_width_kind == _WP_from_arg) __width = __format::__int_from_arg(__ctx.arg(_M_width));
      return __width;
    }

    template<typename _Context>
    constexpr size_t _M_get_precision(_Context& __ctx) const {
      size_t __prec = -1;
      if (_M_prec_kind == _WP_value) __prec = _M_prec;
      else if (_M_prec_kind == _WP_from_arg) __prec = __format::__int_from_arg(__ctx.arg(_M_prec));
      return __prec;
    }
  };

  template<typename _Int>
  inline constexpr char* __put_sign(_Int __i, _Sign __sign, char* __dest) noexcept {
    if (__i < 0) *__dest = '-';
    else if (__sign == _Sign_plus) *__dest = '+';
    else if (__sign == _Sign_space) *__dest = ' ';
    else ++__dest;
    return __dest;
  }

  template<typename _Out, typename _CharT>
    requires output_iterator<_Out, const _CharT&>
  inline constexpr _Out __write(_Out __out, basic_string_view<_CharT> __str) {
    if constexpr (is_same_v<_Out, _Sink_iter<_CharT>>) {
      if (__str.size()) __out = __str;
    } else
      for (_CharT __c : __str) *__out++ = __c;
    return __out;
  }

  template<typename _Out, typename _CharT>
  constexpr _Out
  __write_padded(_Out __out, basic_string_view<_CharT> __str, _Align __align, size_t __nfill, char32_t __fill_char) {
    const size_t __buflen = 0x20;
    _CharT __padding_chars[__buflen];
    __padding_chars[0] = _CharT();
    basic_string_view<_CharT> __padding{__padding_chars, __buflen};

    auto __pad = [&__padding](size_t __n, _Out& __o) {
      if (__n == 0) return;
      while (__n > __padding.size()) {
        __o = __format::__write(std::move(__o), __padding);
        __n -= __padding.size();
      }
      if (__n != 0) __o = __format::__write(std::move(__o), __padding.substr(0, __n));
    };

    size_t __l, __r, __max;
    if (__align == _Align_centre) {
      __l = __nfill / 2;
      __r = __l + (__nfill & 1);
      __max = __r;
    } else if (__align == _Align_right) {
      __l = __nfill;
      __r = 0;
      __max = __l;
    } else {
      __l = 0;
      __r = __nfill;
      __max = __r;
    }

    using namespace __unicode;
    if constexpr (__literal_encoding_is_unicode<_CharT>())
      if (!__is_single_code_unit<_CharT>(__fill_char)) [[unlikely]] {
        const char32_t __arr[1]{__fill_char};
        _Utf_view<_CharT, span<const char32_t, 1>> __v(__arr);
        basic_string<_CharT> __padstr(__v.begin(), __v.end());
        __padding = __padstr;
        while (__l-- > 0) __out = __format::__write(std::move(__out), __padding);
        __out = __format::__write(std::move(__out), __str);
        while (__r-- > 0) __out = __format::__write(std::move(__out), __padding);
        return __out;
      }

    if (__max < __buflen) __padding.remove_suffix(__buflen - __max);
    else __max = __buflen;

    char_traits<_CharT>::assign(__padding_chars, __max, __fill_char);
    __pad(__l, __out);
    __out = __format::__write(std::move(__out), __str);
    __pad(__r, __out);

    return __out;
  }

  template<typename _CharT, typename _Out>
  constexpr _Out __write_padded_as_spec(
    basic_string_view<type_identity_t<_CharT>> __str, size_t __estimated_width,
    basic_format_context<_Out, _CharT>& __fc, const _Spec<_CharT>& __spec, _Align __align = _Align_left
  ) {
    size_t __width = __spec._M_get_width(__fc);

    if (__width <= __estimated_width) return __format::__write(__fc.out(), __str);

    const size_t __nfill = __width - __estimated_width;

    if (__spec._M_align != _Align_default) __align = __spec._M_align;

    return __format::__write_padded(__fc.out(), __str, __align, __nfill, __spec._M_fill);
  }

  template<typename _CharT>
  constexpr size_t __truncate(basic_string_view<_CharT>& __s, size_t __prec) {
    if constexpr (__unicode::__literal_encoding_is_unicode<_CharT>()) {
      if (__prec != (size_t)-1) return __unicode::__truncate(__s, __prec);
      else return __unicode::__field_width(__s);
    } else {
      __s = __s.substr(0, __prec);
      return __s.size();
    }
  }

  enum class _Term_char : unsigned char {
    _Term_none,
    _Term_quote,
    _Term_apos,
  };
  using enum _Term_char;

  template<typename _CharT>
  struct _Escapes {
    using _Str_view = basic_string_view<_CharT>;

    static consteval _Str_view _S_all() { return _GLIBCXX_WIDEN("\t\\t\n\\n\r\\r\\\\\\\"\\\"'\\'\\u\\x"); }

    static consteval _Str_view _S_tab() { return _S_all().substr(0, 3); }

    static consteval _Str_view _S_newline() { return _S_all().substr(3, 3); }

    static consteval _Str_view _S_return() { return _S_all().substr(6, 3); }

    static consteval _Str_view _S_bslash() { return _S_all().substr(9, 3); }

    static consteval _Str_view _S_quote() { return _S_all().substr(12, 3); }

    static consteval _Str_view _S_apos() { return _S_all().substr(15, 3); }

    static consteval _Str_view _S_u() { return _S_all().substr(18, 2); }

    static consteval _Str_view _S_x() { return _S_all().substr(20, 2); }

    static constexpr _Str_view _S_term(_Term_char __term) {
      switch (__term) {
      case _Term_none:
        return _Str_view();
      case _Term_quote:
        return _S_quote().substr(0, 1);
      case _Term_apos:
        return _S_apos().substr(0, 1);
      }
      __builtin_unreachable();
    }
  };

  template<typename _CharT>
  struct _Separators {
    using _Str_view = basic_string_view<_CharT>;

    static consteval _Str_view _S_all() { return _GLIBCXX_WIDEN("[]{}(), : "); }

    static consteval _Str_view _S_squares() { return _S_all().substr(0, 2); }

    static consteval _Str_view _S_braces() { return _S_all().substr(2, 2); }

    static consteval _Str_view _S_parens() { return _S_all().substr(4, 2); }

    static consteval _Str_view _S_comma() { return _S_all().substr(6, 2); }

    static consteval _Str_view _S_colon() { return _S_all().substr(8, 2); }
  };

  template<typename _CharT>
  constexpr bool __should_escape_ascii(_CharT __c, _Term_char __term) {
    using _Esc = _Escapes<_CharT>;
    switch (__c) {
    case _Esc::_S_tab()[0]:
    case _Esc::_S_newline()[0]:
    case _Esc::_S_return()[0]:
    case _Esc::_S_bslash()[0]:
      return true;
    case _Esc::_S_quote()[0]:
      return __term == _Term_quote;
    case _Esc::_S_apos()[0]:
      return __term == _Term_apos;
    default:
      return (__c >= 0 && __c < 0x20) || __c == 0x7f;
    };
  }

  constexpr bool __should_escape_unicode(char32_t __c, bool __prev_esc) {
    if (__unicode::__should_escape_category(__c)) return __c != U' ';
    if (!__prev_esc) return false;
    return __unicode::__grapheme_cluster_break_property(__c) == __unicode::_Gcb_property::_Gcb_Extend;
  }

  using uint_least32_t = __UINT_LEAST32_TYPE__;
  template<typename _Out, typename _CharT>
  constexpr _Out __write_escape_seq(_Out __out, uint_least32_t __val, basic_string_view<_CharT> __prefix) {
    constexpr size_t __max = 8;
    char __buf[__max];
    const string_view __narrow(__buf, std::__to_chars_i<uint_least32_t>(__buf, __buf + __max, __val, 16).ptr);

    __out = __format::__write(__out, __prefix);
    *__out = _Separators<_CharT>::_S_braces()[0];
    ++__out;
    if constexpr (is_same_v<char, _CharT>) __out = __format::__write(__out, __narrow);
    *__out = _Separators<_CharT>::_S_braces()[1];
    return ++__out;
  }

  template<typename _Out, typename _CharT>
  constexpr _Out __write_escape_seqs(_Out __out, basic_string_view<_CharT> __units) {
    using _UChar = make_unsigned_t<_CharT>;
    for (_CharT __c : __units)
      __out = __format::__write_escape_seq(__out, static_cast<_UChar>(__c), _Escapes<_CharT>::_S_x());
    return __out;
  }

  template<typename _Out, typename _CharT>
  constexpr _Out __write_escaped_char(_Out __out, _CharT __c) {
    using _UChar = make_unsigned_t<_CharT>;
    using _Esc = _Escapes<_CharT>;
    switch (__c) {
    case _Esc::_S_tab()[0]:
      return __format::__write(__out, _Esc::_S_tab().substr(1, 2));
    case _Esc::_S_newline()[0]:
      return __format::__write(__out, _Esc::_S_newline().substr(1, 2));
    case _Esc::_S_return()[0]:
      return __format::__write(__out, _Esc::_S_return().substr(1, 2));
    case _Esc::_S_bslash()[0]:
      return __format::__write(__out, _Esc::_S_bslash().substr(1, 2));
    case _Esc::_S_quote()[0]:
      return __format::__write(__out, _Esc::_S_quote().substr(1, 2));
    case _Esc::_S_apos()[0]:
      return __format::__write(__out, _Esc::_S_apos().substr(1, 2));
    default:
      return __format::__write_escape_seq(__out, static_cast<_UChar>(__c), _Esc::_S_u());
    }
  }

  template<typename _CharT, typename _Out>
  constexpr _Out __write_escaped_ascii(_Out __out, basic_string_view<_CharT> __str, _Term_char __term) {
    using _Str_view = basic_string_view<_CharT>;
    if consteval {
      constexpr _Str_view __supported(
        _GLIBCXX_WIDEN(
          "ABCDEFGHIJKLMNOPQRSTUWXYZ"
          "abdeefghijklmnopqrstuwzyz"
          " !#$%&'()*+-./:;<=>?[]^_{|}~"
          "0123456789"
          "\t\n\r\\\"\'\0"
        ),
        95
      );
      if (__str.find_first_not_of(__supported) != _Str_view::npos)
#if __has_builtin(__builtin_constexpr_diag)
        __builtin_constexpr_diag(
          2, "",
          "for non-Unicode literal encodings, only"
          " printable ASCII characters and standard"
          " escape sequencess can be escaped in constant"
          " expressions"
        );
#else
        __asm__("");
#endif
    }

    auto __first = __str.begin();
    auto const __last = __str.end();
    while (__first != __last) {
      auto __print = __first;

      while (__print != __last && !__format::__should_escape_ascii(*__print, __term)) ++__print;

      if (__print != __first) __out = __format::__write(__out, _Str_view(__first, __print));

      if (__print == __last) return __out;

      __first = __print;
      __out = __format::__write_escaped_char(__out, *__first);
      ++__first;
    }
    return __out;
  }

  template<typename _CharT, typename _Out>
  constexpr _Out
  __write_escaped_unicode_part(_Out __out, basic_string_view<_CharT>& __str, bool& __prev_esc, _Term_char __term) {
    using _Str_view = basic_string_view<_CharT>;
    using _Esc = _Escapes<_CharT>;

    static constexpr char32_t __replace = U'\uFFFD';
    static constexpr _Str_view __replace_rep = [] {
      if constexpr (is_same_v<char, _CharT>) return "\xEF\xBF\xBD";
      else return L"\xFFFD";
    }();

    __unicode::_Utf_view<char32_t, _Str_view> __v(std::move(__str));
    __str = {};

    auto __first = __v.begin();
    auto const __last = __v.end();
    while (__first != __last) {
      bool __esc_ascii = false;
      bool __esc_unicode = false;
      bool __esc_replace = false;
      auto __should_escape = [&](auto const& __it) {
        if (*__it <= 0x7f) return __esc_ascii = __format::__should_escape_ascii(*__it.base(), __term);
        if (__format::__should_escape_unicode(*__it, __prev_esc)) return __esc_unicode = true;
        if (*__it == __replace) {
          _Str_view __units(__it.base(), __it._M_units());
          return __esc_replace = (__units != __replace_rep);
        }
        return false;
      };

      auto __print = __first;
      while (__print != __last && !__should_escape(__print)) {
        __prev_esc = false;
        ++__print;
      }

      if (__print != __first) __out = __format::__write(__out, _Str_view(__first.base(), __print.base()));

      if (__print == __last) return __out;

      __first = __print;
      if (__esc_ascii) __out = __format::__write_escaped_char(__out, *__first.base());
      else if (__esc_unicode) __out = __format::__write_escape_seq(__out, *__first, _Esc::_S_u());

      else if (_Str_view __units(__first.base(), __first._M_units()); __units.end() != __last.base())
        __out = __format::__write_escape_seqs(__out, __units);
      else {
        __str = __units;
        return __out;
      }

      __prev_esc = true;
      ++__first;
    }

    return __out;
  }

  template<typename _CharT, typename _Out>
  constexpr _Out __write_escaped_unicode(_Out __out, basic_string_view<_CharT> __str, _Term_char __term) {
    bool __prev_escape = true;
    __out = __format::__write_escaped_unicode_part(__out, __str, __prev_escape, __term);
    __out = __format::__write_escape_seqs(__out, __str);
    return __out;
  }

  template<typename _CharT, typename _Out>
  constexpr _Out __write_escaped(_Out __out, basic_string_view<_CharT> __str, _Term_char __term) {
    __out = __format::__write(__out, _Escapes<_CharT>::_S_term(__term));

    if constexpr (__unicode::__literal_encoding_is_unicode<_CharT>())
      __out = __format::__write_escaped_unicode(__out, __str, __term);
    else if constexpr (is_same_v<char, _CharT> && __unicode::__literal_encoding_is_extended_ascii())
      __out = __format::__write_escaped_ascii(__out, __str, __term);
    else __out = __format::__write_escaped_ascii(__out, __str, __term);

    return __format::__write(__out, _Escapes<_CharT>::_S_term(__term));
  }

  template<__char _CharT>
  struct __formatter_str {
    __formatter_str() = default;

    constexpr __formatter_str(_Spec<_CharT> __spec) noexcept : _M_spec(__spec) {}

    constexpr typename basic_format_parse_context<_CharT>::iterator parse(basic_format_parse_context<_CharT>& __pc) {
      auto __first = __pc.begin();
      const auto __last = __pc.end();
      _Spec<_CharT> __spec{};

      auto __finalize = [this, &__spec] { _M_spec = __spec; };

      auto __finished = [&] {
        if (__first == __last || *__first == '}') {
          __finalize();
          return true;
        }
        return false;
      };

      if (__finished()) return __first;

      __first = __spec._M_parse_fill_and_align(__first, __last);
      if (__finished()) return __first;

      __first = __spec._M_parse_width(__first, __last, __pc);
      if (__finished()) return __first;

      __first = __spec._M_parse_precision(__first, __last, __pc);
      if (__finished()) return __first;

      if (*__first == 's') {
        __spec._M_type = _Pres_s;
        ++__first;
      }
#if __glibcxx_format_ranges
      else if (*__first == '?') {
        __spec._M_debug = true;
        ++__first;
      }
#endif

      if (__finished()) return __first;

      __format::__failed_to_parse_format_spec();
    }

    template<typename _Out>
    constexpr _Out format(basic_string_view<_CharT> __s, basic_format_context<_Out, _CharT>& __fc) const {
      if (_M_spec._M_debug) return _M_format_escaped(__s, __fc);

      if (_M_spec._M_width_kind == _WP_none && _M_spec._M_prec_kind == _WP_none)
        return __format::__write(__fc.out(), __s);

      const size_t __maxwidth = _M_spec._M_get_precision(__fc);
      const size_t __width = __format::__truncate(__s, __maxwidth);
      return __format::__write_padded_as_spec(__s, __width, __fc, _M_spec);
    }

    template<typename _Out>
    constexpr _Out _M_format_escaped(basic_string_view<_CharT> __s, basic_format_context<_Out, _CharT>& __fc) const {
      const size_t __padwidth = _M_spec._M_get_width(__fc);
      if (__padwidth == 0 && _M_spec._M_prec_kind == _WP_none)
        return __format::__write_escaped(__fc.out(), __s, _Term_quote);

      const size_t __maxwidth = _M_spec._M_get_precision(__fc);
      const size_t __width = __truncate(__s, __maxwidth);

      if (__padwidth <= __width && _M_spec._M_prec_kind == _WP_none)
        return __format::__write_escaped(__fc.out(), __s, _Term_quote);

      _Padding_sink<_Out, _CharT> __sink(__fc.out(), __padwidth, __maxwidth);
      __format::__write_escaped(__sink.out(), __s, _Term_quote);
      return __sink._M_finish(_M_spec._M_align, _M_spec._M_fill);
    }

#if __glibcxx_format_ranges
    template<ranges::input_range _Rg, typename _Out>
      requires same_as<remove_cvref_t<ranges::range_reference_t<_Rg>>, _CharT>
    constexpr _Out _M_format_range(_Rg&& __rg, basic_format_context<_Out, _CharT>& __fc) const {
      using _Range = remove_reference_t<_Rg>;
      using _String_view = basic_string_view<_CharT>;
      if constexpr (ranges::contiguous_range<_Rg>) {
        _String_view __str(ranges::data(__rg), size_t(ranges::distance(__rg)));
        return format(__str, __fc);
      } else if constexpr (!is_const_v<_Range> && __simply_formattable_range<_Range, _CharT>)
        return _M_format_range<const _Range&>(__rg, __fc);
      else if constexpr (!is_lvalue_reference_v<_Rg>) return _M_format_range<_Range&>(__rg, __fc);
      else {
        auto __handle_debug = [this, &__rg]<typename _NOut>(_NOut __nout) {
          if (!_M_spec._M_debug) return ranges::copy(__rg, std::move(__nout)).out;

          _Escaping_sink<_NOut, _CharT> __sink(std::move(__nout), _Term_quote);
          ranges::copy(__rg, __sink.out());
          return __sink._M_finish();
        };

        const size_t __padwidth = _M_spec._M_get_width(__fc);
        if (__padwidth == 0 && _M_spec._M_prec_kind == _WP_none) return __handle_debug(__fc.out());

        _Padding_sink<_Out, _CharT> __sink(__fc.out(), __padwidth, _M_spec._M_get_precision(__fc));
        __handle_debug(__sink.out());
        return __sink._M_finish(_M_spec._M_align, _M_spec._M_fill);
      }
    }

    constexpr void set_debug_format() noexcept { _M_spec._M_debug = true; }
#endif

  private:
    _Spec<_CharT> _M_spec{};
  };

  [[__gnu__::__always_inline__]]
  constexpr char __toupper_numeric(char __c) {
    switch (__c) {
    case 'a':
      return 'A';
    case 'b':
      return 'B';
    case 'c':
      return 'C';
    case 'd':
      return 'D';
    case 'e':
      return 'E';
    case 'f':
      return 'F';
    case 'i':
      return 'I';
    case 'n':
      return 'N';
    case 'p':
      return 'P';
    case 'x':
      return 'X';
    default:
      return __c;
    }
  }

#undef _GLIBCXX_FORMAT_F128

  using std::to_chars;

  template<typename _Tp>
  concept __formattable_float = is_same_v<remove_cv_t<_Tp>, _Tp> && requires(_Tp __t, char* __p) {
    __format::to_chars(__p, __p, __t, chars_format::scientific, 6);
  };

} // namespace __format

template<__format::__char _CharT>
struct formatter<_CharT, _CharT> {
  formatter() = default;

  constexpr typename basic_format_parse_context<_CharT>::iterator parse(basic_format_parse_context<_CharT>& __pc) {
    throw;
  }

  template<typename _Out>
  constexpr typename basic_format_context<_Out, _CharT>::iterator
  format(_CharT __u, basic_format_context<_Out, _CharT>& __fc) const {
    throw;
  }

#if __glibcxx_format_ranges
  constexpr void set_debug_format() noexcept { terminate(); }
#endif
};

template<__format::__char _CharT>
struct formatter<const _CharT*, _CharT> {
  formatter() = default;

  [[__gnu__::__always_inline__]]
  constexpr typename basic_format_parse_context<_CharT>::iterator parse(basic_format_parse_context<_CharT>& __pc) {
    return _M_f.parse(__pc);
  }

  template<typename _Out>
  [[__gnu__::__nonnull__]]
  constexpr typename basic_format_context<_Out, _CharT>::iterator
  format(const _CharT* __u, basic_format_context<_Out, _CharT>& __fc) const {
    return _M_f.format(__u, __fc);
  }

#if __glibcxx_format_ranges
  constexpr void set_debug_format() noexcept { _M_f.set_debug_format(); }
#endif

private:
  __format::__formatter_str<_CharT> _M_f;
};

namespace __format {

  template<typename _Tp>
  constexpr bool __is_formattable_integer = __is_integer<_Tp>::__value;

  template<>
  inline constexpr bool __is_formattable_integer<char> = false;
  template<>
  inline constexpr bool __is_formattable_integer<wchar_t> = false;
#ifdef _GLIBCXX_USE_CHAR8_T
  template<>
  inline constexpr bool __is_formattable_integer<char8_t> = false;
#endif
  template<>
  inline constexpr bool __is_formattable_integer<char16_t> = false;
  template<>
  inline constexpr bool __is_formattable_integer<char32_t> = false;

  template<typename _Tp>
  concept __formattable_integer = __is_formattable_integer<_Tp>;
} // namespace __format

template<__format::__formattable_integer _Tp, __format::__char _CharT>
struct formatter<_Tp, _CharT> {
  formatter() = default;

  [[__gnu__::__always_inline__]]
  constexpr typename basic_format_parse_context<_CharT>::iterator parse(basic_format_parse_context<_CharT>& __pc) {
    throw;
  }

  template<typename _Out>
  constexpr typename basic_format_context<_Out, _CharT>::iterator
  format(_Tp __u, basic_format_context<_Out, _CharT>& __fc) const {
    throw;
  }
};

#if defined __glibcxx_to_chars

template<__format::__formattable_float _Tp, __format::__char _CharT>
struct formatter<_Tp, _CharT> {
  formatter() = default;

  [[__gnu__::__always_inline__]]
  constexpr typename basic_format_parse_context<_CharT>::iterator parse(basic_format_parse_context<_CharT>& __pc) {
    throw;
  }

  template<typename _Out>
  typename basic_format_context<_Out, _CharT>::iterator
  format(_Tp __u, basic_format_context<_Out, _CharT>& __fc) const {
    throw;
  }
};

#endif

/** Format a pointer.
 * @{
 */
template<__format::__char _CharT>
struct formatter<const void*, _CharT> {
  formatter() = default;

  constexpr typename basic_format_parse_context<_CharT>::iterator parse(basic_format_parse_context<_CharT>& __pc) {
    throw;
  }

  template<typename _Out>
  constexpr typename basic_format_context<_Out, _CharT>::iterator
  format(const void* __v, basic_format_context<_Out, _CharT>& __fc) const {
    throw;
  }
};

template<typename _Out>
struct format_to_n_result {
  _Out out;
  iter_difference_t<_Out> size;
};

_GLIBCXX_BEGIN_NAMESPACE_CONTAINER
template<typename, typename>
class vector;
_GLIBCXX_END_NAMESPACE_CONTAINER

namespace __format {
  template<typename _CharT>
  class _Drop_iter {
  public:
    using iterator_category = output_iterator_tag;
    using value_type = void;
    using difference_type = ptrdiff_t;
    using pointer = void;
    using reference = void;

    _Drop_iter() = default;
    _Drop_iter(const _Drop_iter&) = default;
    _Drop_iter& operator=(const _Drop_iter&) = default;

    [[__gnu__::__always_inline__]]
    constexpr _Drop_iter& operator=(_CharT __c) {
      return *this;
    }

    [[__gnu__::__always_inline__]]
    constexpr _Drop_iter& operator=(basic_string_view<_CharT> __s) {
      return *this;
    }

    [[__gnu__::__always_inline__]]
    constexpr _Drop_iter& operator*() {
      return *this;
    }

    [[__gnu__::__always_inline__]]
    constexpr _Drop_iter& operator++() {
      return *this;
    }

    [[__gnu__::__always_inline__]]
    constexpr _Drop_iter operator++(int) {
      return *this;
    }
  };

  template<typename _CharT>
  class _Sink_iter {
    _Sink<_CharT>* _M_sink = nullptr;

  public:
    using iterator_category = output_iterator_tag;
    using value_type = void;
    using difference_type = ptrdiff_t;
    using pointer = void;
    using reference = void;

    _Sink_iter() = default;
    _Sink_iter(const _Sink_iter&) = default;
    _Sink_iter& operator=(const _Sink_iter&) = default;

    [[__gnu__::__always_inline__]]
    explicit constexpr _Sink_iter(_Sink<_CharT>& __sink)
        : _M_sink(std::addressof(__sink)) {}

    [[__gnu__::__always_inline__]]
    constexpr _Sink_iter& operator=(_CharT __c) {
      _M_sink->_M_write(__c);
      return *this;
    }

    [[__gnu__::__always_inline__]]
    constexpr _Sink_iter& operator=(basic_string_view<_CharT> __s) {
      _M_sink->_M_write(__s);
      return *this;
    }

    [[__gnu__::__always_inline__]]
    constexpr _Sink_iter& operator*() {
      return *this;
    }

    [[__gnu__::__always_inline__]]
    constexpr _Sink_iter& operator++() {
      return *this;
    }

    [[__gnu__::__always_inline__]]
    constexpr _Sink_iter operator++(int) {
      return *this;
    }

    constexpr auto _M_reserve(size_t __n) const { return _M_sink->_M_reserve(__n); }

    constexpr bool _M_discarding() const { return _M_sink->_M_discarding(); }
  };

  template<typename _CharT>
  class _Sink {
    friend class _Sink_iter<_CharT>;

    span<_CharT> _M_span;
    typename span<_CharT>::iterator _M_next;

    virtual void _M_overflow() = 0;

  protected:
    [[__gnu__::__always_inline__]]
    explicit constexpr _Sink(span<_CharT> __span) noexcept
        : _M_span(__span), _M_next(__span.begin()) {}

    [[__gnu__::__always_inline__]]
    constexpr span<_CharT> _M_used() const noexcept {
      return _M_span.first(_M_next - _M_span.begin());
    }

    [[__gnu__::__always_inline__]]
    constexpr span<_CharT> _M_unused() const noexcept {
      return _M_span.subspan(_M_next - _M_span.begin());
    }

    [[__gnu__::__always_inline__]]
    constexpr void _M_rewind() noexcept {
      _M_next = _M_span.begin();
    }

    constexpr void _M_reset(span<_CharT> __s, size_t __pos = 0) noexcept {
      _M_span = __s;
      _M_next = __s.begin() + __pos;
    }

    constexpr void _M_write(_CharT __c) {
      *_M_next++ = __c;
      if (_M_next - _M_span.begin() == std::ssize(_M_span)) [[unlikely]]
        _M_overflow();
    }

    constexpr void _M_write(basic_string_view<_CharT> __s) {
      span __to = _M_unused();
      while (__to.size() <= __s.size()) {
        __s.copy(__to.data(), __to.size());
        _M_next += __to.size();
        __s.remove_prefix(__to.size());
        _M_overflow();
        __to = _M_unused();
      }
      if (__s.size()) {
        __s.copy(__to.data(), __s.size());
        _M_next += __s.size();
      }
    }

    struct _Reservation {
      constexpr explicit operator bool() const noexcept { return _M_sink; }

      constexpr _CharT* get() const noexcept { return _M_sink->_M_next.operator->(); }

      constexpr void _M_bump(size_t __n) { _M_sink->_M_bump(__n); }
      _Sink* _M_sink;
    };

    constexpr virtual _Reservation _M_reserve(size_t __n) {
      if (__n <= _M_unused().size()) return {this};

      if (__n <= _M_span.size()) {
        _M_overflow();
        if (__n <= _M_unused().size()) return {this};
      }
      return {nullptr};
    }

    constexpr virtual void _M_bump(size_t __n) { _M_next += __n; }

    constexpr virtual bool _M_discarding() const { return false; }

  public:
    _Sink(const _Sink&) = delete;
    _Sink& operator=(const _Sink&) = delete;

    [[__gnu__::__always_inline__]]
    constexpr _Sink_iter<_CharT> out() noexcept {
      return _Sink_iter<_CharT>(*this);
    }
  };

  template<typename _CharT>
  class _Fixedbuf_sink final : public _Sink<_CharT> {
    constexpr void _M_overflow() override {
      __glibcxx_assert(false);
      this->_M_rewind();
    }

  public:
    [[__gnu__::__always_inline__]]
    constexpr explicit _Fixedbuf_sink(span<_CharT> __buf)
        : _Sink<_CharT>(__buf) {}

    constexpr basic_string_view<_CharT> view() const {
      auto __s = this->_M_used();
      return basic_string_view<_CharT>(__s.data(), __s.size());
    }
  };

  template<typename _CharT>
  class _Buf_sink : public _Sink<_CharT> {
  protected:
    _CharT _M_buf[__stackbuf_size<_CharT>];

    [[__gnu__::__always_inline__]]
    constexpr _Buf_sink() noexcept
        : _Sink<_CharT>(_M_buf) {}
  };

  using _GLIBCXX_STD_C::vector;

  template<typename _Seq>
  class _Seq_sink : public _Buf_sink<typename _Seq::value_type> {
    using _CharT = typename _Seq::value_type;

    _Seq _M_seq;

  protected:
    constexpr void _M_overflow() override {
      auto __s = this->_M_used();
      if (__s.empty()) [[unlikely]]
        return;

      _GLIBCXX_DEBUG_ASSERT(__s.data() != _M_seq.data());

      if constexpr (__is_specialization_of<_Seq, basic_string>) _M_seq.append(__s.data(), __s.size());
      else _M_seq.insert(_M_seq.end(), __s.begin(), __s.end());

      this->_M_rewind();
    }

    constexpr typename _Sink<_CharT>::_Reservation _M_reserve(size_t __n) override {
      if constexpr (__is_specialization_of<_Seq, basic_string> || __is_specialization_of<_Seq, vector>) {
        if (this->_M_used().size()) [[unlikely]]
          _Seq_sink::_M_overflow();

        const auto __sz = _M_seq.size();
        if constexpr (is_same_v<string, _Seq> || is_same_v<wstring, _Seq>)
          _M_seq.__resize_and_overwrite(__sz + __n, [](auto, auto __n2) { return __n2; });
        else _M_seq.resize(__sz + __n);

        this->_M_reset(_M_seq, __sz);
        return {this};
      } else return _Sink<_CharT>::_M_reserve(__n);
    }

    constexpr void _M_bump(size_t __n) override {
      if constexpr (__is_specialization_of<_Seq, basic_string> || __is_specialization_of<_Seq, vector>) {
        auto __s = this->_M_used();
        _GLIBCXX_DEBUG_ASSERT(__s.data() == _M_seq.data());

        _M_seq.resize(__s.size() + __n);

        this->_M_reset(this->_M_buf);
      }
    }

    constexpr void _M_trim(span<const _CharT> __s)
      requires __is_specialization_of<_Seq, basic_string>
    {
      _GLIBCXX_DEBUG_ASSERT(__s.data() == this->_M_buf || __s.data() == _M_seq.data());
      if (__s.data() == _M_seq.data()) _M_seq.resize(__s.size());
      else this->_M_reset(this->_M_buf, __s.size());
    }

  public:
    [[__gnu__::__always_inline__]]
    constexpr _Seq_sink() noexcept(is_nothrow_default_constructible_v<_Seq>) {}

    constexpr _Seq_sink(_Seq&& __s) noexcept(is_nothrow_move_constructible_v<_Seq>) : _M_seq(std::move(__s)) {}

    using _Sink<_CharT>::out;

    constexpr _Seq get() && {
      if (this->_M_used().size() != 0) _Seq_sink::_M_overflow();
      return std::move(_M_seq);
    }

    constexpr span<_CharT> _M_span() {
      auto __s = this->_M_used();
      if (_M_seq.size()) {
        if (__s.size() != 0) _Seq_sink::_M_overflow();
        return _M_seq;
      }
      return __s;
    }

    constexpr basic_string_view<_CharT> view() {
      auto __span = _M_span();
      return basic_string_view<_CharT>(__span.data(), __span.size());
    }
  };

  template<typename _CharT, typename _Alloc = allocator<_CharT>>
  using _Str_sink = _Seq_sink<basic_string<_CharT, char_traits<_CharT>, _Alloc>>;

  template<typename _CharT, typename _OutIter>
  class _Iter_sink : public _Buf_sink<_CharT> {
    _OutIter _M_out;
    iter_difference_t<_OutIter> _M_max;

  protected:
    size_t _M_count = 0;

    constexpr void _M_overflow() override {
      auto __s = this->_M_used();
      if (_M_max < 0) _M_out = ranges::copy(__s, std::move(_M_out)).out;
      else if (_M_count < static_cast<size_t>(_M_max)) {
        auto __max = _M_max - _M_count;
        span<_CharT> __first;
        if (__max < __s.size()) __first = __s.first(static_cast<size_t>(__max));
        else __first = __s;
        _M_out = ranges::copy(__first, std::move(_M_out)).out;
      }
      this->_M_rewind();
      _M_count += __s.size();
    }

    constexpr bool _M_discarding() const override { return false; }

  public:
    [[__gnu__::__always_inline__]]
    constexpr explicit _Iter_sink(_OutIter __out, iter_difference_t<_OutIter> __max = -1)
        : _M_out(std::move(__out)), _M_max(__max) {}

    using _Sink<_CharT>::out;

    constexpr format_to_n_result<_OutIter> _M_finish() && {
      if (this->_M_used().size() != 0) _Iter_sink::_M_overflow();
      iter_difference_t<_OutIter> __count(_M_count);
      return {std::move(_M_out), __count};
    }
  };

  template<typename _CharT>
  class _Ptr_sink : public _Sink<_CharT> {
    static constexpr size_t _S_no_limit = size_t(-1);

    size_t _M_max;

  protected:
    size_t _M_count = 0;

  private:
    _CharT _M_buf[64];

  protected:
    constexpr void _M_overflow() override {
      if (this->_M_unused().size() != 0) return;

      auto __s = this->_M_used();

      if (_M_max != _S_no_limit) {
        _M_count += __s.size();

        this->_M_reset(this->_M_buf);
      } else {
        _M_rebuf(__s.data(), __s.size() + 1024, __s.size());
      }
    }

    constexpr bool _M_discarding() const override { return false; }

    constexpr typename _Sink<_CharT>::_Reservation _M_reserve(size_t __n) final {
      auto __avail = this->_M_unused();
      if (__n > __avail.size()) {
        if (_M_max != _S_no_limit) return {};

        auto __s = this->_M_used();
        _M_rebuf(__s.data(), __s.size() + __n, __s.size());
      }
      return {this};
    }

  private:
    template<typename _IterDifference>
    static constexpr size_t _S_trim_max(_IterDifference __max) {
      if (__max < 0) return _S_no_limit;
      if constexpr (!is_integral_v<_IterDifference> || sizeof(__max) > sizeof(size_t))

        if (_IterDifference((size_t)-1) < __max) return _S_no_limit;
      return size_t(__max);
    }

    [[__gnu__::__always_inline__]]
    constexpr void _M_rebuf(_CharT* __ptr, size_t __total, size_t __inuse = 0) {
      std::span<_CharT> __span(__ptr, __total);
      this->_M_reset(__span, __inuse);
    }

  public:
    constexpr explicit _Ptr_sink(_CharT* __ptr, size_t __n = _S_no_limit) noexcept
        : _Sink<_CharT>(_M_buf), _M_max(__n) {
      if (__n == 0) return;
      else if (__n != _S_no_limit) _M_rebuf(__ptr, __n);
#if __has_builtin(__builtin_dynamic_object_size)
      else if (size_t __bytes = __builtin_dynamic_object_size(__ptr, 2)) _M_rebuf(__ptr, __bytes / sizeof(_CharT));
#endif
      else {
        const auto __off = reinterpret_cast<__UINTPTR_TYPE__>(__ptr) % 1024;
        __n = (1024 - __off) / sizeof(_CharT);
        if (__n > 0) [[likely]]
          _M_rebuf(__ptr, __n);
        else _M_rebuf(__ptr, 1);
      }
    }

    template<contiguous_iterator _OutIter>
    constexpr explicit _Ptr_sink(_OutIter __out, iter_difference_t<_OutIter> __n = -1)
        : _Ptr_sink(std::to_address(__out), _S_trim_max(__n)) {}

    template<contiguous_iterator _OutIter>
    constexpr format_to_n_result<_OutIter> _M_finish(_OutIter __first) const {
      auto __s = this->_M_used();
      if (__s.data() == _M_buf) {
        iter_difference_t<_OutIter> __m(_M_max);
        iter_difference_t<_OutIter> __count(_M_count + __s.size());
        return {__first + __m, __count};
      } else {
        iter_difference_t<_OutIter> __count(__s.size());
        return {__first + __count, __count};
      }
    }
  };

  template<typename _CharT, typename _OutIter>
  concept __contiguous_char_iter = contiguous_iterator<_OutIter> && same_as<iter_value_t<_OutIter>, _CharT>;

  template<typename _Out, typename _CharT>
  class _Padding_sink : public _Str_sink<_CharT> {
    size_t _M_padwidth;
    size_t _M_maxwidth;
    _Out _M_out;
    size_t _M_printwidth;

    [[__gnu__::__always_inline__]]
    constexpr bool _M_ignoring() const {
      return _M_printwidth >= _M_maxwidth;
    }

    [[__gnu__::__always_inline__]]
    constexpr bool _M_buffering() const {
      if (_M_printwidth < _M_padwidth) return true;
      if (_M_maxwidth != (size_t)-1) return _M_printwidth < _M_maxwidth;
      return false;
    }

    constexpr void _M_sync_discarding() {
      if constexpr (is_same_v<_Out, _Sink_iter<_CharT>>)
        if (_M_out._M_discarding()) _M_maxwidth = _M_printwidth;
    }

    constexpr void _M_flush() {
      span<_CharT> __new = this->_M_used();
      basic_string_view<_CharT> __str(__new.data(), __new.size());
      _M_out = __format::__write(std::move(_M_out), __str);
      _M_sync_discarding();
      this->_M_rewind();
    }

    constexpr bool _M_force_update() {
      auto __str = this->view();

      _M_printwidth = __format::__truncate(__str, _M_maxwidth);
      if (_M_ignoring()) this->_M_trim(__str);
      if (_M_buffering()) return true;

      if (_M_printwidth >= _M_padwidth) {
        _M_out = __format::__write(std::move(_M_out), __str);
        _M_sync_discarding();
      }

      else
        _Str_sink<_CharT>::_M_overflow();

      this->_M_reset(this->_M_buf);
      return false;
    }

    constexpr bool _M_update(size_t __new) {
      _M_printwidth += __new;

      if (_M_printwidth >= _M_padwidth || _M_printwidth >= _M_maxwidth) return _M_force_update();
      return true;
    }

    constexpr void _M_overflow() override {
      if (_M_ignoring()) this->_M_rewind();

      else if (!_M_buffering()) _M_flush();

      else if (_M_update(this->_M_used().size())) _Str_sink<_CharT>::_M_overflow();
    }

    constexpr bool _M_discarding() const override { return _M_ignoring(); }

    constexpr typename _Sink<_CharT>::_Reservation _M_reserve(size_t __n) override {
      if (_M_ignoring()) this->_M_rewind();
      else if constexpr (is_same_v<_Out, _Sink_iter<_CharT>>)
        if (!_M_buffering()) {
          if (!this->_M_used().empty()) _M_flush();

          if (auto __reserved = _M_out._M_reserve(__n)) return __reserved;
        }
      return _Sink<_CharT>::_M_reserve(__n);
    }

    constexpr void _M_bump(size_t __n) override {
      if (_M_ignoring()) return;

      _Sink<_CharT>::_M_bump(__n);
      if (_M_buffering()) _M_update(__n);
    }

  public:
    [[__gnu__::__always_inline__]]
    constexpr explicit _Padding_sink(_Out __out, size_t __padwidth, size_t __maxwidth)
        : _M_padwidth(__padwidth), _M_maxwidth(__maxwidth), _M_out(std::move(__out)), _M_printwidth(0) {
      _M_sync_discarding();
    }

    [[__gnu__::__always_inline__]]
    constexpr explicit _Padding_sink(_Out __out, size_t __padwidth)
        : _Padding_sink(std::move(__out), __padwidth, (size_t)-1) {}

    constexpr _Out _M_finish(_Align __align, char32_t __fill_char) {
      if (auto __rem = this->_M_used().size()) {
        if (_M_ignoring()) this->_M_rewind();
        else if (!_M_buffering()) _M_flush();
        else _M_update(__rem);
      }

      if (!_M_buffering() || !_M_force_update())

        if (_M_printwidth >= _M_padwidth) return std::move(_M_out);

      const auto __str = this->view();
      if (_M_printwidth >= _M_padwidth) return __format::__write(std::move(_M_out), __str);

      const size_t __nfill = _M_padwidth - _M_printwidth;
      return __format::__write_padded(std::move(_M_out), __str, __align, __nfill, __fill_char);
    }
  };

  template<typename _Out, typename _CharT>
  class _Escaping_sink : public _Buf_sink<_CharT> {
    using _Esc = _Escapes<_CharT>;

    _Out _M_out;
    _Term_char _M_term : 2;
    unsigned _M_prev_escape : 1;
    unsigned _M_out_discards : 1;

    constexpr void _M_sync_discarding() {
      if constexpr (is_same_v<_Out, _Sink_iter<_CharT>>) _M_out_discards = _M_out._M_discarding();
    }

    constexpr void _M_write() {
      span<_CharT> __bytes = this->_M_used();
      basic_string_view<_CharT> __str(__bytes.data(), __bytes.size());

      size_t __rem = 0;
      if constexpr (__unicode::__literal_encoding_is_unicode<_CharT>()) {
        bool __prev_escape = _M_prev_escape;
        _M_out = __format::__write_escaped_unicode_part(std::move(_M_out), __str, __prev_escape, _M_term);
        _M_prev_escape = __prev_escape;

        __rem = __str.size();
        if (__rem > 0 && __str.data() != this->_M_buf) [[unlikely]]
          ranges::move(__str, this->_M_buf);
      } else _M_out = __format::__write_escaped_ascii(std::move(_M_out), __str, _M_term);

      this->_M_reset(this->_M_buf, __rem);
      _M_sync_discarding();
    }

    constexpr void _M_overflow() override {
      if (_M_out_discards) this->_M_rewind();
      else _M_write();
    }

    constexpr bool _M_discarding() const override { return _M_out_discards; }

  public:
    [[__gnu__::__always_inline__]]
    constexpr explicit _Escaping_sink(_Out __out, _Term_char __term)
        : _M_out(std::move(__out)), _M_term(__term), _M_prev_escape(true), _M_out_discards(false) {
      _M_out = __format::__write(std::move(_M_out), _Esc::_S_term(_M_term));
      _M_sync_discarding();
    }

    constexpr _Out _M_finish() {
      if (_M_out_discards) return std::move(_M_out);

      if (!this->_M_used().empty()) {
        _M_write();
        if constexpr (__unicode::__literal_encoding_is_unicode<_CharT>())
          if (auto __rem = this->_M_used(); !__rem.empty()) {
            basic_string_view<_CharT> __str(__rem.data(), __rem.size());
            _M_out = __format::__write_escape_seqs(std::move(_M_out), __str);
          }
      }
      return __format::__write(std::move(_M_out), _Esc::_S_term(_M_term));
    }
  };

  enum class _Arg_t : unsigned char {
    _Arg_none,
    _Arg_bool,
    _Arg_c,
    _Arg_i,
    _Arg_u,
    _Arg_ll,
    _Arg_ull,
    _Arg_flt,
    _Arg_dbl,
    _Arg_ldbl,
    _Arg_str,
    _Arg_sv,
    _Arg_ptr,
    _Arg_handle,
    _Arg_i128,
    _Arg_u128,
    _Arg_float128,
    _Arg_bf16,
    _Arg_f16,
    _Arg_f32,
    _Arg_f64,
    _Arg_max_,

  };
  using enum _Arg_t;

  template<typename _Context>
  struct _Arg_value {
    using _CharT = typename _Context::char_type;

    class handle {
      using _CharT = typename _Context::char_type;
      using _Func = void (*)(basic_format_parse_context<_CharT>&, _Context&, const void*);

      template<typename _Tp>
      using __maybe_const_t = __conditional_t<__formattable_with<const _Tp, _Context>, const _Tp, _Tp>;

      template<typename _Tq>
      static constexpr void
      _S_format(basic_format_parse_context<_CharT>& __parse_ctx, _Context& __format_ctx, const void* __ptr) {
        using _Td = remove_const_t<_Tq>;
        typename _Context::template formatter_type<_Td> __f;
        __parse_ctx.advance_to(__f.parse(__parse_ctx));
        _Tq& __val = *const_cast<_Tq*>(static_cast<const _Td*>(__ptr));
        __format_ctx.advance_to(__f.format(__val, __format_ctx));
      }

      template<typename _Tp>
        requires(!is_same_v<remove_cv_t<_Tp>, handle>)
      explicit constexpr handle(_Tp& __val) noexcept
          : _M_ptr(__builtin_addressof(__val)), _M_func(&_S_format<__maybe_const_t<_Tp>>) {}

      friend class basic_format_arg<_Context>;

    public:
      handle(const handle&) = default;
      handle& operator=(const handle&) = default;

      [[__gnu__::__always_inline__]]
      void constexpr format(basic_format_parse_context<_CharT>& __pc, _Context& __fc) const {
        _M_func(__pc, __fc, this->_M_ptr);
      }

    private:
      const void* _M_ptr;
      _Func _M_func;
    };

    union {
      monostate _M_none;
      bool _M_bool;
      _CharT _M_c;
      int _M_i;
      unsigned _M_u;
      long long _M_ll;
      unsigned long long _M_ull;
      float _M_flt;
      double _M_dbl;
      long double _M_ldbl;
      const _CharT* _M_str;
      basic_string_view<_CharT> _M_sv;
      const void* _M_ptr;
      handle _M_handle;
    };

    [[__gnu__::__always_inline__]]
    constexpr _Arg_value()
        : _M_none() {}

#if 0
      template<typename _Tp>
	constexpr
	_Arg_value(in_place_type_t<_Tp>, _Tp __val)
	{ _S_get<_Tp>() = __val; }
#endif

    template<typename _Tp, typename _Self, typename... _Value>
    [[__gnu__::__always_inline__]]
    static constexpr auto& _S_access(_Self& __u, _Value... __value) noexcept {
      static_assert(sizeof...(_Value) <= 1);
      if constexpr (is_same_v<_Tp, bool>) return (__u._M_bool = ... = __value);
      else if constexpr (is_same_v<_Tp, _CharT>) return (__u._M_c = ... = __value);
      else if constexpr (is_same_v<_Tp, int>) return (__u._M_i = ... = __value);
      else if constexpr (is_same_v<_Tp, unsigned>) return (__u._M_u = ... = __value);
      else if constexpr (is_same_v<_Tp, long long>) return (__u._M_ll = ... = __value);
      else if constexpr (is_same_v<_Tp, unsigned long long>) return (__u._M_ull = ... = __value);
      else if constexpr (is_same_v<_Tp, float>) return (__u._M_flt = ... = __value);
      else if constexpr (is_same_v<_Tp, double>) return (__u._M_dbl = ... = __value);
      else if constexpr (is_same_v<_Tp, long double>) return (__u._M_ldbl = ... = __value);
      else if constexpr (is_same_v<_Tp, const _CharT*>) return (__u._M_str = ... = __value);
      else if constexpr (is_same_v<_Tp, basic_string_view<_CharT>>) return (__u._M_sv = ... = __value);
      else if constexpr (is_same_v<_Tp, const void*>) return (__u._M_ptr = ... = __value);
      else if constexpr (is_same_v<_Tp, handle>) return __u._M_handle;

      __builtin_unreachable();
    }

    template<typename _Tp>
    [[__gnu__::__always_inline__]]
    constexpr auto& _M_get() noexcept {
      return _S_access<_Tp>(*this);
    }

    template<typename _Tp>
    [[__gnu__::__always_inline__]]
    constexpr const auto& _M_get() const noexcept {
      return _S_access<_Tp>(*this);
    }

    template<typename _Tp>
    [[__gnu__::__always_inline__]]
    constexpr void _M_set(_Tp __v) noexcept {
      if constexpr (is_same_v<_Tp, basic_string_view<_CharT>>) std::construct_at(&_M_sv, __v);
      else if constexpr (is_same_v<_Tp, handle>) std::construct_at(&_M_handle, __v);
      else _S_access<_Tp>(*this, __v);
    }
  };

  template<typename _Context, typename... _Args>
  class _Arg_store;

  template<typename _Visitor, typename _Ctx>
  constexpr decltype(auto) __visit_format_arg(_Visitor&&, basic_format_arg<_Ctx>);

  template<typename _Ch, typename _Tp>
  consteval _Arg_t __to_arg_t_enum() noexcept;
} // namespace __format

template<typename _Context>
class basic_format_arg {
  using _CharT = typename _Context::char_type;

public:
  using handle = __format::_Arg_value<_Context>::handle;

  [[__gnu__::__always_inline__]]
  constexpr basic_format_arg() noexcept
      : _M_type(__format::_Arg_none) {}

  [[nodiscard, __gnu__::__always_inline__]]
  explicit constexpr operator bool() const noexcept {
    return _M_type != __format::_Arg_none;
  }

#if __cpp_lib_format >= 202306L
  template<typename _Visitor>
  constexpr decltype(auto) visit(this basic_format_arg __arg, _Visitor&& __vis) {
    return __arg._M_visit_user(std::forward<_Visitor>(__vis));
  }

  template<typename _Res, typename _Visitor>
  constexpr _Res visit(this basic_format_arg __arg, _Visitor&& __vis) {
    return __arg._M_visit_user(std::forward<_Visitor>(__vis));
  }
#endif

private:
  template<typename _Ctx>
  friend class basic_format_args;

  template<typename _Ctx, typename... _Args>
  friend class __format::_Arg_store;

  static_assert(is_trivially_copyable_v<__format::_Arg_value<_Context>>);

  __format::_Arg_value<_Context> _M_val;
  __format::_Arg_t _M_type;

  template<typename _Tp>
  static consteval auto _S_to_arg_type() {
    using _Td = remove_const_t<_Tp>;
    if constexpr (is_same_v<_Td, bool>) return type_identity<bool>();
    else if constexpr (is_same_v<_Td, _CharT>) return type_identity<_CharT>();
    else if constexpr (is_same_v<_Td, char> && is_same_v<_CharT, wchar_t>) return type_identity<_CharT>();
    else if constexpr (__is_signed_integer<_Td>::value) {
      if constexpr (sizeof(_Td) <= sizeof(int)) return type_identity<int>();
      else if constexpr (sizeof(_Td) <= sizeof(long long)) return type_identity<long long>();
    } else if constexpr (__is_unsigned_integer<_Td>::value) {
      if constexpr (sizeof(_Td) <= sizeof(unsigned)) return type_identity<unsigned>();
      else if constexpr (sizeof(_Td) <= sizeof(unsigned long long)) return type_identity<unsigned long long>();
    } else if constexpr (is_same_v<_Td, float>) return type_identity<float>();
    else if constexpr (is_same_v<_Td, double>) return type_identity<double>();
    else if constexpr (is_same_v<_Td, long double>) return type_identity<long double>();
    else if constexpr (__is_specialization_of<_Td, basic_string_view> || __is_specialization_of<_Td, basic_string>) {
      if constexpr (is_same_v<typename _Td::value_type, _CharT>) return type_identity<basic_string_view<_CharT>>();
      else return type_identity<handle>();
    } else if constexpr (is_same_v<decay_t<_Td>, const _CharT*>) return type_identity<const _CharT*>();
    else if constexpr (is_same_v<decay_t<_Td>, _CharT*>) return type_identity<const _CharT*>();
    else if constexpr (is_void_v<remove_pointer_t<_Td>>) return type_identity<const void*>();
    else if constexpr (is_same_v<_Td, nullptr_t>) return type_identity<const void*>();
    else return type_identity<handle>();
  }

  template<typename _Tp>
  using _Normalize = typename decltype(_S_to_arg_type<_Tp>())::type;

  template<typename _Tp>
  static consteval __format::_Arg_t _S_to_enum() {
    using namespace __format;
    if constexpr (is_same_v<_Tp, bool>) return _Arg_bool;
    else if constexpr (is_same_v<_Tp, _CharT>) return _Arg_c;
    else if constexpr (is_same_v<_Tp, int>) return _Arg_i;
    else if constexpr (is_same_v<_Tp, unsigned>) return _Arg_u;
    else if constexpr (is_same_v<_Tp, long long>) return _Arg_ll;
    else if constexpr (is_same_v<_Tp, unsigned long long>) return _Arg_ull;
    else if constexpr (is_same_v<_Tp, float>) return _Arg_flt;
    else if constexpr (is_same_v<_Tp, double>) return _Arg_dbl;
    else if constexpr (is_same_v<_Tp, long double>) return _Arg_ldbl;
    else if constexpr (is_same_v<_Tp, const _CharT*>) return _Arg_str;
    else if constexpr (is_same_v<_Tp, basic_string_view<_CharT>>) return _Arg_sv;
    else if constexpr (is_same_v<_Tp, const void*>) return _Arg_ptr;
    else if constexpr (is_same_v<_Tp, handle>) return _Arg_handle;
  }

  template<typename _Tp>
  constexpr void _M_set(_Tp __v) noexcept {
    _M_type = _S_to_enum<_Tp>();
    _M_val._M_set(__v);
  }

  template<typename _Tp>
    requires __format::__formattable_with<_Tp, _Context>
  constexpr explicit basic_format_arg(_Tp& __v) noexcept {
    using _Td = _Normalize<_Tp>;
    if constexpr (is_same_v<_Td, basic_string_view<_CharT>>) _M_set(_Td{__v.data(), __v.size()});
    else if constexpr (is_same_v<remove_const_t<_Tp>, char> && is_same_v<_CharT, wchar_t>)
      _M_set(static_cast<_Td>(static_cast<unsigned char>(__v)));
    else _M_set(static_cast<_Td>(__v));
  }

  template<typename _Ctx, typename... _Argz>
  friend constexpr auto make_format_args(_Argz&...) noexcept;

  template<typename _Visitor, typename _Ctx>
  friend constexpr decltype(auto) visit_format_arg(_Visitor&& __vis, basic_format_arg<_Ctx>);

  template<typename _Visitor, typename _Ctx>
  friend constexpr decltype(auto) __format::__visit_format_arg(_Visitor&&, basic_format_arg<_Ctx>);

  template<typename _Ch, typename _Tp>
  friend consteval __format::_Arg_t __format::__to_arg_t_enum() noexcept;

  [[__gnu__::__noinline__]]
  handle _M_handle_unrecognized() const;

  template<typename _Visitor>
  constexpr decltype(auto) _M_visit(_Visitor&& __vis) {
    switch (_M_type) {
      using enum __format::_Arg_t;
    case _Arg_none:
      return std::forward<_Visitor>(__vis)(_M_val._M_none);
    case _Arg_bool:
      return std::forward<_Visitor>(__vis)(_M_val._M_bool);
    case _Arg_c:
      return std::forward<_Visitor>(__vis)(_M_val._M_c);
    case _Arg_i:
      return std::forward<_Visitor>(__vis)(_M_val._M_i);
    case _Arg_u:
      return std::forward<_Visitor>(__vis)(_M_val._M_u);
    case _Arg_ll:
      return std::forward<_Visitor>(__vis)(_M_val._M_ll);
    case _Arg_ull:
      return std::forward<_Visitor>(__vis)(_M_val._M_ull);
#if __glibcxx_to_chars
    case _Arg_flt:
      return std::forward<_Visitor>(__vis)(_M_val._M_flt);
    case _Arg_dbl:
      return std::forward<_Visitor>(__vis)(_M_val._M_dbl);
    case _Arg_ldbl:
      return std::forward<_Visitor>(__vis)(_M_val._M_ldbl);
#endif
    case _Arg_str:
      return std::forward<_Visitor>(__vis)(_M_val._M_str);
    case _Arg_sv:
      return std::forward<_Visitor>(__vis)(_M_val._M_sv);
    case _Arg_ptr:
      return std::forward<_Visitor>(__vis)(_M_val._M_ptr);
    case _Arg_handle:
      return std::forward<_Visitor>(__vis)(_M_val._M_handle);
    default:

      handle __h = _M_handle_unrecognized();
      return std::forward<_Visitor>(__vis)(__h);
    }
  }

  template<typename _Visitor>
  constexpr decltype(auto) _M_visit_user(_Visitor&& __vis) {
    return _M_visit([&__vis]<typename _Tp>(_Tp& __val) -> decltype(auto) {
      constexpr bool __user_facing = __is_one_of<
        _Tp, monostate, bool, _CharT, int, unsigned int, long long int, unsigned long long int, float, double,
        long double, const _CharT*, basic_string_view<_CharT>, const void*, handle>::value;
      if constexpr (__user_facing) return std::forward<_Visitor>(__vis)(__val);
      else {
        handle __h(__val);
        return std::forward<_Visitor>(__vis)(__h);
      }
    });
  }
};

template<typename _Visitor, typename _Context>
_GLIBCXX26_DEPRECATED_SUGGEST("std::basic_format_arg::visit")
inline constexpr decltype(auto) visit_format_arg(_Visitor&& __vis, basic_format_arg<_Context> __arg) {
  return __arg._M_visit_user(std::forward<_Visitor>(__vis));
}

namespace __format {
  template<typename _Visitor, typename _Ctx>
  inline constexpr decltype(auto) __visit_format_arg(_Visitor&& __vis, basic_format_arg<_Ctx> __arg) {
    return __arg._M_visit(std::forward<_Visitor>(__vis));
  }

  struct _WidthPrecVisitor {
    template<typename _Tp>
    constexpr size_t operator()(_Tp& __arg) const {
      if constexpr (is_same_v<_Tp, monostate>) __format::__invalid_arg_id_in_format_string();

      else if constexpr (sizeof(_Tp) <= sizeof(long long)) {
        if constexpr (__is_unsigned_integer<_Tp>::value) return __arg;
        else if constexpr (__is_signed_integer<_Tp>::value)
          if (__arg >= 0) return __arg;
      }
      __throw_format_error(
        "format error: argument used for width or "
        "precision must be a non-negative integer"
      );
    }
  };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  template<typename _Context>
  inline constexpr size_t __int_from_arg(const basic_format_arg<_Context>& __arg) {
    return __format::__visit_format_arg(_WidthPrecVisitor(), __arg);
  }

  template<int _Bits, size_t _Nm>
  constexpr auto __pack_arg_types(const array<_Arg_t, _Nm>& __types) {
    __UINT64_TYPE__ __packed_types = 0;
    for (auto __i = __types.rbegin(); __i != __types.rend(); ++__i)
      __packed_types = (__packed_types << _Bits) | (unsigned)*__i;
    return __packed_types;
  }
} // namespace __format

template<typename _Context>
class basic_format_args {
  static constexpr int _S_packed_type_bits = 5;
  static constexpr int _S_packed_type_mask = 0b11111;
  static constexpr int _S_max_packed_args = 12;

  static_assert((unsigned)__format::_Arg_max_ <= (1u << _S_packed_type_bits));

  template<typename... _Args>
  using _Store = __format::_Arg_store<_Context, _Args...>;

  template<typename _Ctx, typename... _Args>
  friend class __format::_Arg_store;

  using uint64_t = __UINT64_TYPE__;
  using _Format_arg = basic_format_arg<_Context>;
  using _Format_arg_val = __format::_Arg_value<_Context>;

  uint64_t _M_packed_size : 4;
  uint64_t _M_unpacked_size : 60;

  union {
    const _Format_arg_val* _M_values;
    const _Format_arg* _M_args;
  };

  constexpr size_t _M_size() const noexcept { return _M_packed_size ? _M_packed_size : _M_unpacked_size; }

  constexpr typename __format::_Arg_t _M_type(size_t __i) const noexcept {
    uint64_t __t = _M_unpacked_size >> (__i * _S_packed_type_bits);
    return static_cast<__format::_Arg_t>(__t & _S_packed_type_mask);
  }

  template<typename _Ctx, typename... _Args>
  friend constexpr auto make_format_args(_Args&...) noexcept;

  template<typename... _Args>
  static consteval array<__format::_Arg_t, sizeof...(_Args)> _S_types_to_pack() {
    return {_Format_arg::template _S_to_enum<_Args>()...};
  }

public:
  template<typename... _Args>
  constexpr basic_format_args(const _Store<_Args...>& __store) noexcept;

  [[nodiscard, __gnu__::__always_inline__]]
  constexpr basic_format_arg<_Context> get(size_t __i) const noexcept {
    basic_format_arg<_Context> __arg;
    if (__i < _M_packed_size) {
      __arg._M_type = _M_type(__i);
      __arg._M_val = _M_values[__i];
    } else if (_M_packed_size == 0 && __i < _M_unpacked_size) __arg = _M_args[__i];
    return __arg;
  }
};

template<typename _Context, typename... _Args>
basic_format_args(__format::_Arg_store<_Context, _Args...>) -> basic_format_args<_Context>;

template<typename _Context, typename... _Args>
constexpr auto make_format_args(_Args&... __fmt_args) noexcept;

template<typename _Context, typename... _Args>
class __format::_Arg_store {
  friend std::basic_format_args<_Context>;

  template<typename _Ctx, typename... _Argz>
  friend constexpr auto std::
#if _GLIBCXX_INLINE_VERSION
    __8::
#endif
      make_format_args(_Argz&...) noexcept;

  static constexpr bool _S_values_only = sizeof...(_Args) <= basic_format_args<_Context>::_S_max_packed_args;

  using _Element_t = __conditional_t<_S_values_only, __format::_Arg_value<_Context>, basic_format_arg<_Context>>;

  _Element_t _M_args[sizeof...(_Args)];

  template<typename _Tp>
  static constexpr _Element_t _S_make_elt(_Tp& __v) {
    using _Tq = remove_const_t<_Tp>;
    using _CharT = typename _Context::char_type;
    static_assert(
      is_default_constructible_v<formatter<_Tq, _CharT>>, "std::formatter must be specialized for the type "
                                                          "of each format arg"
    );
    using __format::__formattable_with;
    if constexpr (is_const_v<_Tp>)
      if constexpr (!__formattable_with<_Tp, _Context>)
        if constexpr (__formattable_with<_Tq, _Context>)
          static_assert(
            __formattable_with<_Tp, _Context>, "format arg must be non-const because its "
                                               "std::formatter specialization has a "
                                               "non-const reference parameter"
          );
    basic_format_arg<_Context> __arg(__v);
    if constexpr (_S_values_only) return __arg._M_val;
    else return __arg;
  }

  template<typename... _Tp>
    requires(sizeof...(_Tp) == sizeof...(_Args))
  [[__gnu__::__always_inline__]] constexpr _Arg_store(_Tp&... __a) noexcept : _M_args{_S_make_elt(__a)...} {}
};

template<typename _Context>
class __format::_Arg_store<_Context> {};

template<typename _Context>
template<typename... _Args>
inline constexpr basic_format_args<_Context>::basic_format_args(const _Store<_Args...>& __store) noexcept {
  if constexpr (sizeof...(_Args) == 0) {
    _M_packed_size = 0;
    _M_unpacked_size = 0;
    _M_args = nullptr;
  } else if constexpr (sizeof...(_Args) <= _S_max_packed_args) {
    _M_packed_size = sizeof...(_Args);

    _M_unpacked_size = __format::__pack_arg_types<_S_packed_type_bits>(_S_types_to_pack<_Args...>());

    _M_values = __store._M_args;
  } else {
    _M_packed_size = 0;

    _M_unpacked_size = sizeof...(_Args);

    _M_args = __store._M_args;
  }
}

template<typename _Context = format_context, typename... _Args>
[[nodiscard, __gnu__::__always_inline__]]
inline constexpr auto make_format_args(_Args&... __fmt_args) noexcept {
  using _Fmt_arg = basic_format_arg<_Context>;
  using _Store = __format::_Arg_store<_Context, typename _Fmt_arg::template _Normalize<_Args>...>;
  return _Store(__fmt_args...);
}

namespace __format {
  template<typename _Out, typename _CharT, typename _Context>
  constexpr _Out __do_vformat_to(_Out, basic_string_view<_CharT>, const basic_format_args<_Context>&);

  template<typename _CharT>
  struct __formatter_chrono;

} // namespace __format

/** Context for std::format and similar functions.
 *
 * A formatting context contains an output iterator and locale to use
 * for the formatting operations. Most programs will never need to use
 * this class template explicitly. For typical uses of `std::format` the
 * library will use the specializations `std::format_context` (for `char`)
 * and `std::wformat_context` (for `wchar_t`).
 *
 * You are not allowed to define partial or explicit specializations of
 * this class template.
 *
 * @since C++20
 */
template<typename _Out, typename _CharT>

class _GLIBCXX_NO_SPECIALIZATIONS basic_format_context {
  static_assert(output_iterator<_Out, const _CharT&>);

  basic_format_args<basic_format_context> _M_args;
  _Out _M_out;

  constexpr basic_format_context(basic_format_args<basic_format_context> __args, _Out __out)
      : _M_args(__args), _M_out(std::move(__out)) {}

  basic_format_context(const basic_format_context&) = delete;
  basic_format_context& operator=(const basic_format_context&) = delete;

  template<typename _Out2, typename _CharT2, typename _Context2>
  friend constexpr _Out2
  __format::__do_vformat_to(_Out2, basic_string_view<_CharT2>, const basic_format_args<_Context2>&);

  friend __format::__formatter_chrono<_CharT>;

public:
  ~basic_format_context() = default;

  using iterator = _Out;
  using char_type = _CharT;
  template<typename _Tp>
  using formatter_type = formatter<_Tp, _CharT>;

  [[nodiscard]]
  constexpr basic_format_arg<basic_format_context> arg(size_t __id) const noexcept {
    return _M_args.get(__id);
  }

  [[nodiscard]]
  constexpr iterator out() {
    return std::move(_M_out);
  }

  constexpr void advance_to(iterator __it) { _M_out = std::move(__it); }
};

#if _GLIBCXX_EXTERN_TEMPLATE

extern template basic_format_arg<format_context>::handle
basic_format_arg<format_context>::_M_handle_unrecognized() const;
#else
template<typename _Context>
typename basic_format_arg<_Context>::handle basic_format_arg<_Context>::_M_handle_unrecognized() const {
  __throw_format_error("format error: unrecognized argument type");
}
#endif

namespace __format {

  template<typename _CharT>
  struct _Scanner {
    using iterator = typename basic_format_parse_context<_CharT>::iterator;

    struct _Parse_context : basic_format_parse_context<_CharT> {
      using basic_format_parse_context<_CharT>::basic_format_parse_context;
      const _Arg_t* _M_types = nullptr;
    } _M_pc;

    constexpr explicit _Scanner(basic_string_view<_CharT> __str, size_t __nargs = (size_t)-1) : _M_pc(__str, __nargs) {}

    constexpr iterator begin() const noexcept { return _M_pc.begin(); }
    constexpr iterator end() const noexcept { return _M_pc.end(); }

    constexpr void _M_scan() {
      basic_string_view<_CharT> __fmt = _M_fmt_str();

      if (__fmt.size() == 2 && __fmt[0] == '{' && __fmt[1] == '}') {
        _M_pc.advance_to(begin() + 1);
        _M_format_arg(_M_pc.next_arg_id());
        return;
      }

      size_t __lbr = __fmt.find('{');
      size_t __rbr = __fmt.find('}');

      while (__fmt.size()) {
        auto __cmp = __lbr <=> __rbr;
        if (__cmp == 0) {
          _M_on_chars(end());
          _M_pc.advance_to(end());
          return;
        } else if (__cmp < 0) {
          if (__lbr + 1 == __fmt.size() || (__rbr == __fmt.npos && __fmt[__lbr + 1] != '{'))
            __format::__unmatched_left_brace_in_format_string();
          const bool __is_escape = __fmt[__lbr + 1] == '{';
          iterator __last = begin() + __lbr + int(__is_escape);
          _M_on_chars(__last);
          _M_pc.advance_to(__last + 1);
          __fmt = _M_fmt_str();
          if (__is_escape) {
            if (__rbr != __fmt.npos) __rbr -= __lbr + 2;
            __lbr = __fmt.find('{');
          } else {
            _M_on_replacement_field();
            __fmt = _M_fmt_str();
            __lbr = __fmt.find('{');
            __rbr = __fmt.find('}');
          }
        } else {
          if (++__rbr == __fmt.size() || __fmt[__rbr] != '}') __format::__unmatched_right_brace_in_format_string();
          iterator __last = begin() + __rbr;
          _M_on_chars(__last);
          _M_pc.advance_to(__last + 1);
          __fmt = _M_fmt_str();
          if (__lbr != __fmt.npos) __lbr -= __rbr + 1;
          __rbr = __fmt.find('}');
        }
      }
    }

    constexpr basic_string_view<_CharT> _M_fmt_str() const noexcept { return {begin(), end()}; }

    constexpr virtual void _M_on_chars(iterator) {}

    constexpr void _M_on_replacement_field() {
      auto __next = begin();

      size_t __id;
      if (*__next == '}') __id = _M_pc.next_arg_id();
      else if (*__next == ':') {
        __id = _M_pc.next_arg_id();
        _M_pc.advance_to(++__next);
      } else {
        auto [__i, __ptr] = __format::__parse_arg_id(begin(), end());
        if (!__ptr || !(*__ptr == '}' || *__ptr == ':')) __format::__invalid_arg_id_in_format_string();
        _M_pc.check_arg_id(__id = __i);
        if (*__ptr == ':') {
          _M_pc.advance_to(++__ptr);
        } else _M_pc.advance_to(__ptr);
      }
      _M_format_arg(__id);
      if (begin() == end() || *begin() != '}') __format::__unmatched_left_brace_in_format_string();
      _M_pc.advance_to(begin() + 1);
    }

    constexpr virtual void _M_format_arg(size_t __id) = 0;
  };

  template<typename _Out, typename _CharT>
  class _Formatting_scanner : public _Scanner<_CharT> {
  public:
    constexpr _Formatting_scanner(basic_format_context<_Out, _CharT>& __fc, basic_string_view<_CharT> __str)
        : _Scanner<_CharT>(__str), _M_fc(__fc) {}

  private:
    basic_format_context<_Out, _CharT>& _M_fc;

    using iterator = typename _Scanner<_CharT>::iterator;

    constexpr void _M_on_chars(iterator __last) override {
      basic_string_view<_CharT> __str(this->begin(), __last);
      _M_fc.advance_to(__format::__write(_M_fc.out(), __str));
    }

    constexpr void _M_format_arg(size_t __id) override {
      using _Context = basic_format_context<_Out, _CharT>;
      using handle = typename basic_format_arg<_Context>::handle;

      __format::__visit_format_arg(
        [this](auto& __arg) {
          using _Type = remove_reference_t<decltype(__arg)>;
          using _Formatter = typename _Context::template formatter_type<_Type>;
          if constexpr (is_same_v<_Type, monostate>) __format::__invalid_arg_id_in_format_string();
          else if constexpr (is_same_v<_Type, handle>) __arg.format(this->_M_pc, this->_M_fc);
          else if constexpr (is_default_constructible_v<_Formatter>) {
            _Formatter __f;
            this->_M_pc.advance_to(__f.parse(this->_M_pc));
            this->_M_fc.advance_to(__f.format(__arg, this->_M_fc));
          } else static_assert(__format::__formattable_with<_Type, _Context>);
        },
        _M_fc.arg(__id)
      );
    }
  };

  template<typename _CharT, typename _Tp>
  consteval _Arg_t __to_arg_t_enum() noexcept {
    using _Context = __format::__format_context<_CharT>;
    using _Fmt_arg = basic_format_arg<_Context>;
    using _NormalizedTp = typename _Fmt_arg::template _Normalize<_Tp>;
    return _Fmt_arg::template _S_to_enum<_NormalizedTp>();
  }

  template<typename _CharT, typename... _Args>
  class _Checking_scanner : public _Scanner<_CharT> {
    static_assert(
      (is_default_constructible_v<formatter<_Args, _CharT>> && ...),
      "std::formatter must be specialized for each type being formatted"
    );

  public:
    consteval _Checking_scanner(basic_string_view<_CharT> __str) : _Scanner<_CharT>(__str, sizeof...(_Args)) {
#if __cpp_lib_format >= 202305L
      this->_M_pc._M_types = _M_types.data();
#endif
    }

  private:
    constexpr void _M_format_arg(size_t __id) override {
      if constexpr (sizeof...(_Args) != 0) {
        if (__id < sizeof...(_Args)) {
          _M_parse_format_spec<_Args...>(__id);
          return;
        }
      }
      __builtin_unreachable();
    }

    template<typename _Tp, typename... _OtherArgs>
    constexpr void _M_parse_format_spec(size_t __id) {
      if (__id == 0) {
        formatter<_Tp, _CharT> __f;
        this->_M_pc.advance_to(__f.parse(this->_M_pc));
      } else if constexpr (sizeof...(_OtherArgs) != 0) _M_parse_format_spec<_OtherArgs...>(__id - 1);
      else __builtin_unreachable();
    }

#if __cpp_lib_format >= 202305L
    array<_Arg_t, sizeof...(_Args)> _M_types{{ __format::__to_arg_t_enum<_CharT, _Args>()... }};
#endif
  };

  template<typename _CharT, unsigned = __unicode::__literal_encoding_is_unicode<_CharT>()>
  constexpr _Sink_iter<_CharT>
  __do_vformat_to(_Sink_iter<_CharT> __out, basic_string_view<_CharT> __fmt, __format_context<_CharT>& __ctx) {
    if constexpr (is_same_v<_CharT, char>)

      if (__fmt.size() == 2 && __fmt[0] == '{' && __fmt[1] == '}') {
        bool __done = false;
        __format::__visit_format_arg(
          [&](auto& __arg) {
            using _Tp = remove_cvref_t<decltype(__arg)>;
            if constexpr (is_same_v<_Tp, bool>) {
              size_t __len = 4 + !__arg;
              const char* __chars[] = {"false", "true"};
              if (auto __res = __out._M_reserve(__len)) {
                ranges::copy_n(__chars[__arg], __len, __res.get());
                __res._M_bump(__len);
                __done = true;
              }
            } else if constexpr (is_same_v<_Tp, char>) {
              if (auto __res = __out._M_reserve(1)) {
                *__res.get() = __arg;
                __res._M_bump(1);
                __done = true;
              }
            } else if constexpr (is_integral_v<_Tp>) {
              make_unsigned_t<_Tp> __uval;
              const bool __neg = __arg < 0;
              if (__neg) __uval = make_unsigned_t<_Tp>(~__arg) + 1u;
              else __uval = __arg;
              const auto __n = __detail::__to_chars_len(__uval);
              if (auto __res = __out._M_reserve(__n + __neg)) {
                auto __ptr = __res.get();
                *__ptr = '-';
                __detail::__to_chars_10_impl(__ptr + (int)__neg, __n, __uval);
                __res._M_bump(__n + __neg);
                __done = true;
              }
            } else if constexpr (is_convertible_v<_Tp, string_view>) {
              string_view __sv = __arg;
              if (auto __res = __out._M_reserve(__sv.size())) {
                ranges::copy(__sv, __res.get());
                __res._M_bump(__sv.size());
                __done = true;
              }
            }
          },
          __ctx.arg(0)
        );

        if (__done) return __out;
      }

    _Formatting_scanner<_Sink_iter<_CharT>, _CharT> __scanner(__ctx, __fmt);
    __scanner._M_scan();
    return __out;
  }

#if __cplusplus <= 202002L && _GLIBCXX_EXTERN_TEMPLATE
  extern template _Sink_iter<char> __do_vformat_to<char, 1>(_Sink_iter<char>, string_view, format_context&);
#endif

  template<typename _Out, typename _CharT, typename _Context>
  inline constexpr _Out
  __do_vformat_to(_Out __out, basic_string_view<_CharT> __fmt, const basic_format_args<_Context>& __args) {
    if constexpr (is_same_v<_Out, _Sink_iter<_CharT>>) {
      auto __ctx = _Context(__args, __out);
      return __format::__do_vformat_to(__out, __fmt, __ctx);
    } else if constexpr (__contiguous_char_iter<_CharT, _Out>) {
      _Ptr_sink<_CharT> __sink(__out);
      __format::__do_vformat_to(__sink.out(), __fmt, __args);
      return std::move(__sink)._M_finish(__out).out;
    } else {
      _Iter_sink<_CharT, _Out> __sink(std::move(__out));
      __format::__do_vformat_to(__sink.out(), __fmt, __args);
      return std::move(__sink)._M_finish().out;
    }
  }

#pragma GCC diagnostic pop

} // namespace __format

#if __cpp_lib_format >= 202305L

template<typename _CharT>
template<typename... _Ts>
consteval void basic_format_parse_context<_CharT>::__check_dynamic_spec(size_t __id) noexcept {
  if (__id >= _M_num_args) __format::__invalid_arg_id_in_format_string();
  if constexpr (sizeof...(_Ts) != 0) {
    using _Parse_ctx = __format::_Scanner<_CharT>::_Parse_context;
    auto* __args = static_cast<_Parse_ctx*>(this)->_M_types;

    if (!__args) return;

    auto __arg = __args[__id];
    __format::_Arg_t __types[] = {__format::__to_arg_t_enum<_CharT, _Ts>()...};
    for (auto __t : __types)
      if (__arg == __t) return;
  }
  __invalid_dynamic_spec("arg(id) type does not match");
}

#endif

template<typename _CharT, typename... _Args>
template<typename _Tp>
  requires convertible_to<const _Tp&, basic_string_view<_CharT>>
consteval basic_format_string<_CharT, _Args...>::basic_format_string(const _Tp& __s) noexcept : _M_str(__s) {
  __format::_Checking_scanner<_CharT, remove_cvref_t<_Args>...> __scanner(_M_str);
  __scanner._M_scan();
}

template<typename _Out>
  requires output_iterator<_Out, const char&>
[[__gnu__::__always_inline__]]
inline constexpr _Out vformat_to(_Out __out, string_view __fmt, format_args __args) {
  return __format::__do_vformat_to(std::move(__out), __fmt, __args);
}

[[nodiscard]]
inline constexpr string vformat(string_view __fmt, format_args __args) {
  __format::_Str_sink<char> __buf;
  std::vformat_to(__buf.out(), __fmt, __args);
  return std::move(__buf).get();
}

template<typename... _Args>
[[nodiscard]]
inline constexpr string format(format_string<_Args...> __fmt, _Args&&... __args) {
  return std::vformat(__fmt.get(), std::make_format_args(__args...));
}

#if __glibcxx_format_ranges

template<typename _Tp>
consteval range_format __fmt_kind() {
  using _Ref = ranges::range_reference_t<_Tp>;
  if constexpr (is_same_v<remove_cvref_t<_Ref>, _Tp>) return range_format::disabled;
  else if constexpr (requires { typename _Tp::key_type; }) {
    if constexpr (requires { typename _Tp::mapped_type; }) {
      using _Up = remove_cvref_t<_Ref>;
      if constexpr (__is_pair<_Up>) return range_format::map;
      else if constexpr (__is_specialization_of<_Up, tuple>)
        if constexpr (tuple_size_v<_Up> == 2) return range_format::map;
    }
    return range_format::set;
  } else return range_format::sequence;
}

template<ranges::input_range _Rg>
  requires same_as<_Rg, remove_cvref_t<_Rg>>
constexpr range_format format_kind<_Rg> = __fmt_kind<_Rg>();

namespace __format {
  template<typename _CharT, typename _Out, typename _Callback>
  constexpr typename basic_format_context<_Out, _CharT>::iterator
  __format_padded(basic_format_context<_Out, _CharT>& __fc, const _Spec<_CharT>& __spec, _Callback&& __call) {
    if constexpr (is_same_v<_Out, _Drop_iter<_CharT>>) return __fc.out();
    else {
      static_assert(is_same_v<_Out, _Sink_iter<_CharT>>);

      const size_t __padwidth = __spec._M_get_width(__fc);
      if (__padwidth == 0) return __call(__fc);

      struct _Restore_out {
        constexpr _Restore_out(basic_format_context<_Sink_iter<_CharT>, _CharT>& __fc)
            : _M_ctx(std::addressof(__fc)), _M_out(__fc.out()) {}

        constexpr void _M_disarm() { _M_ctx = nullptr; }

        constexpr ~_Restore_out() {
          if (_M_ctx) _M_ctx->advance_to(_M_out);
        }

      private:
        basic_format_context<_Sink_iter<_CharT>, _CharT>* _M_ctx;
        _Sink_iter<_CharT> _M_out;
      };

      _Restore_out __restore(__fc);
      _Padding_sink<_Sink_iter<_CharT>, _CharT> __sink(__fc.out(), __padwidth);
      __fc.advance_to(__sink.out());
      __call(__fc);
      __fc.advance_to(__sink._M_finish(__spec._M_align, __spec._M_fill));
      __restore._M_disarm();
      return __fc.out();
    }
  }

  template<size_t _Pos, typename _Tp, typename _CharT>
  struct __indexed_formatter_storage {
    constexpr void _M_parse() {
      basic_format_parse_context<_CharT> __pc({});
      if (_M_formatter.parse(__pc) != __pc.end()) __format::__failed_to_parse_format_spec();
    }

    template<typename _Out>
    constexpr void _M_format(
      __maybe_const<_Tp, _CharT>& __elem, basic_format_context<_Out, _CharT>& __fc, basic_string_view<_CharT> __sep
    ) const {
      if constexpr (_Pos != 0) __fc.advance_to(__format::__write(__fc.out(), __sep));
      __fc.advance_to(_M_formatter.format(__elem, __fc));
    }

    [[__gnu__::__always_inline__]]
    constexpr void set_debug_format() {
      if constexpr (__has_debug_format<formatter<_Tp, _CharT>>) _M_formatter.set_debug_format();
    }

  private:
    formatter<_Tp, _CharT> _M_formatter;
  };

  template<typename _CharT, typename... _Tps>
  class __tuple_formatter {
    using _String_view = basic_string_view<_CharT>;
    using _Seps = __format::_Separators<_CharT>;

  public:
    constexpr void set_separator(basic_string_view<_CharT> __sep) noexcept { _M_sep = __sep; }

    constexpr void set_brackets(basic_string_view<_CharT> __open, basic_string_view<_CharT> __close) noexcept {
      _M_open = __open;
      _M_close = __close;
    }

    constexpr typename basic_format_parse_context<_CharT>::iterator parse(basic_format_parse_context<_CharT>& __pc) {
      auto __first = __pc.begin();
      const auto __last = __pc.end();
      __format::_Spec<_CharT> __spec{};

      auto __finished = [&] {
        if (__first != __last && *__first != '}') return false;

        _M_spec = __spec;
        _M_felems._M_parse();
        _M_felems.set_debug_format();
        return true;
      };

      if (__finished()) return __first;

      __first = __spec._M_parse_fill_and_align(__first, __last, "{:");
      if (__finished()) return __first;

      __first = __spec._M_parse_width(__first, __last, __pc);
      if (__finished()) return __first;

      if (*__first == 'n') {
        ++__first;
        _M_open = _M_close = _String_view();
      } else if (*__first == 'm') {
        ++__first;
        if constexpr (sizeof...(_Tps) == 2) {
          _M_sep = _Seps::_S_colon();
          _M_open = _M_close = _String_view();
        } else
          __throw_format_error(
            "format error: 'm' specifier requires range"
            " of pair or tuple of two elements"
          );
      }

      if (__finished()) return __first;

      __format::__failed_to_parse_format_spec();
    }

  protected:
    template<typename _Tuple, typename _Out, size_t... _Ids>
    constexpr typename basic_format_context<_Out, _CharT>::iterator
    _M_format(_Tuple& __tuple, index_sequence<_Ids...>, basic_format_context<_Out, _CharT>& __fc) const {
      return _M_format_elems(std::get<_Ids>(__tuple)..., __fc);
    }

    template<typename _Out>
    constexpr typename basic_format_context<_Out, _CharT>::iterator
    _M_format_elems(__maybe_const<_Tps, _CharT>&... __elems, basic_format_context<_Out, _CharT>& __fc) const {
      return __format::__format_padded(__fc, _M_spec, [this, &__elems...](basic_format_context<_Out, _CharT>& __nfc) {
        __nfc.advance_to(__format::__write(__nfc.out(), _M_open));
        _M_felems._M_format(__elems..., __nfc, _M_sep);
        return __format::__write(__nfc.out(), _M_close);
      });
    }

  private:
    template<size_t... _Ids>
    struct __formatters_storage : __indexed_formatter_storage<_Ids, _Tps, _CharT>... {
      template<size_t _Id, typename _Up>
      using _Base = __indexed_formatter_storage<_Id, _Up, _CharT>;

      constexpr void _M_parse() { (_Base<_Ids, _Tps>::_M_parse(), ...); }

      template<typename _Out>
      constexpr void _M_format(
        __maybe_const<_Tps, _CharT>&... __elems, basic_format_context<_Out, _CharT>& __fc, _String_view __sep
      ) const {
        (_Base<_Ids, _Tps>::_M_format(__elems, __fc, __sep), ...);
      }

      constexpr void set_debug_format() { (_Base<_Ids, _Tps>::set_debug_format(), ...); }
    };

    template<size_t... _Ids>
    static constexpr auto _S_create_storage(index_sequence<_Ids...>) -> __formatters_storage<_Ids...>;
    using _Formatters = decltype(_S_create_storage(index_sequence_for<_Tps...>()));

    _Spec<_CharT> _M_spec{};
    _String_view _M_open = _Seps::_S_parens().substr(0, 1);
    _String_view _M_close = _Seps::_S_parens().substr(1, 1);
    _String_view _M_sep = _Seps::_S_comma();
    _Formatters _M_felems;
  };

  template<typename _Tp>
  concept __is_map_formattable = __is_pair<_Tp> || (__is_tuple_v<_Tp> && tuple_size_v<_Tp> == 2);

} // namespace __format

template<typename _Tp, __format::__char _CharT>
  requires same_as<remove_cvref_t<_Tp>, _Tp> && formattable<_Tp, _CharT>
class range_formatter {
  using _String_view = basic_string_view<_CharT>;
  using _Seps = __format::_Separators<_CharT>;

public:
  constexpr void set_separator(basic_string_view<_CharT> __sep) noexcept { _M_sep = __sep; }

  constexpr void set_brackets(basic_string_view<_CharT> __open, basic_string_view<_CharT> __close) noexcept {
    _M_open = __open;
    _M_close = __close;
  }

  constexpr formatter<_Tp, _CharT>& underlying() noexcept { return _M_fval; }

  constexpr const formatter<_Tp, _CharT>& underlying() const noexcept { return _M_fval; }

  constexpr typename basic_format_parse_context<_CharT>::iterator parse(basic_format_parse_context<_CharT>& __pc) {
    auto __first = __pc.begin();
    const auto __last = __pc.end();
    __format::_Spec<_CharT> __spec{};
    bool __no_brace = false;

    auto __finished = [&] { return __first == __last || *__first == '}'; };

    auto __finalize = [&] {
      _M_spec = __spec;
      return __first;
    };

    auto __parse_val = [&](_String_view __nfs = _String_view()) {
      basic_format_parse_context<_CharT> __npc(__nfs);
      if (_M_fval.parse(__npc) != __npc.end()) __format::__failed_to_parse_format_spec();
      if constexpr (__format::__has_debug_format<formatter<_Tp, _CharT>>) _M_fval.set_debug_format();
      return __finalize();
    };

    if (__finished()) return __parse_val();

    __first = __spec._M_parse_fill_and_align(__first, __last, "{:");
    if (__finished()) return __parse_val();

    __first = __spec._M_parse_width(__first, __last, __pc);
    if (__finished()) return __parse_val();

    if (*__first == '?') {
      ++__first;
      __spec._M_debug = true;
      if (__finished() || *__first != 's')
        __throw_format_error(
          "format error: '?' is allowed only in"
          " combination with 's'"
        );
    }

    if (*__first == 's') {
      ++__first;
      if constexpr (same_as<_Tp, _CharT>) {
        __spec._M_type = __format::_Pres_s;
        if (__finished()) return __finalize();
        __throw_format_error(
          "format error: element format specifier"
          " cannot be provided when 's' specifier is used"
        );
      } else
        __throw_format_error(
          "format error: 's' specifier requires"
          " range of character types"
        );
    }

    if (__finished()) return __parse_val();

    if (*__first == 'n') {
      ++__first;
      _M_open = _M_close = _String_view();
      __no_brace = true;
    }

    if (__finished()) return __parse_val();

    if (*__first == 'm') {
      _String_view __m(__first, 1);
      ++__first;
      if constexpr (__format::__is_map_formattable<_Tp>) {
        _M_sep = _Seps::_S_comma();
        if (!__no_brace) {
          _M_open = _Seps::_S_braces().substr(0, 1);
          _M_close = _Seps::_S_braces().substr(1, 1);
        }
        if (__finished()) return __parse_val(__m);
        __throw_format_error(
          "format error: element format specifier"
          " cannot be provided when 'm' specifier is used"
        );
      } else
        __throw_format_error(
          "format error: 'm' specifier requires"
          " range of pairs or tuples of two elements"
        );
    }

    if (__finished()) return __parse_val();

    if (*__first == ':') {
      __pc.advance_to(++__first);
      __first = _M_fval.parse(__pc);
    }

    if (__finished()) return __finalize();

    __format::__failed_to_parse_format_spec();
  }

  template<ranges::input_range _Rg, typename _Out>
    requires formattable<ranges::range_reference_t<_Rg>, _CharT> &&
             same_as<remove_cvref_t<ranges::range_reference_t<_Rg>>, _Tp>
  constexpr typename basic_format_context<_Out, _CharT>::iterator
  format(_Rg&& __rg, basic_format_context<_Out, _CharT>& __fc) const {
    using _Range = remove_reference_t<_Rg>;
    if constexpr (ranges::contiguous_range<_Rg>) {
      const span<__format::__maybe_const<_Tp, _CharT>> __spn(ranges::data(__rg), size_t(ranges::distance(__rg)));
      return _M_format(__spn, __fc);
    } else if constexpr (__format::__simply_formattable_range<_Range, _CharT>)
      return _M_format<const _Range>(__rg, __fc);
    else return _M_format(__rg, __fc);
  }

private:
  template<ranges::input_range _Rg, typename _Out>
  constexpr typename basic_format_context<_Out, _CharT>::iterator
  _M_format(_Rg& __rg, basic_format_context<_Out, _CharT>& __fc) const {
    if constexpr (same_as<_Tp, _CharT>)
      if (_M_spec._M_type == __format::_Pres_s) {
        __format::__formatter_str __fstr(_M_spec);
        return __fstr._M_format_range(__rg, __fc);
      }
    return __format::__format_padded(__fc, _M_spec, [this, &__rg](basic_format_context<_Out, _CharT>& __nfc) {
      return _M_format_elems(__rg, __nfc);
    });
  }

  template<ranges::input_range _Rg, typename _Out>
  constexpr typename basic_format_context<_Out, _CharT>::iterator
  _M_format_elems(_Rg& __rg, basic_format_context<_Out, _CharT>& __fc) const {
    auto __out = __format::__write(__fc.out(), _M_open);

    auto __first = ranges::begin(__rg);
    auto const __last = ranges::end(__rg);
    if (__first == __last) return __format::__write(__out, _M_close);

    __fc.advance_to(__out);
    __out = _M_fval.format(*__first, __fc);
    for (++__first; __first != __last; ++__first) {
      __out = __format::__write(__out, _M_sep);
      __fc.advance_to(__out);
      __out = _M_fval.format(*__first, __fc);
    }

    return __format::__write(__out, _M_close);
  }

  __format::_Spec<_CharT> _M_spec{};
  _String_view _M_open = _Seps::_S_squares().substr(0, 1);
  _String_view _M_close = _Seps::_S_squares().substr(1, 1);
  _String_view _M_sep = _Seps::_S_comma();
  formatter<_Tp, _CharT> _M_fval;
};

template<ranges::input_range _Rg, __format::__char _CharT>
  requires(format_kind<_Rg> != range_format::disabled) && formattable<ranges::range_reference_t<_Rg>, _CharT>
struct formatter<_Rg, _CharT> {
private:
  static const bool _S_range_format_is_string =
    (format_kind<_Rg> == range_format::string) || (format_kind<_Rg> == range_format::debug_string);
  using _Vt = remove_cvref_t<ranges::range_reference_t<__format::__maybe_const_range<_Rg, _CharT>>>;

  static consteval bool _S_is_correct() {
    if constexpr (_S_range_format_is_string) static_assert(same_as<_Vt, _CharT>);
    return true;
  }

  static_assert(_S_is_correct());

public:
  constexpr formatter() noexcept {
    using _Seps = __format::_Separators<_CharT>;
    if constexpr (format_kind<_Rg> == range_format::map) {
      static_assert(__format::__is_map_formattable<_Vt>);
      _M_under.set_brackets(_Seps::_S_braces().substr(0, 1), _Seps::_S_braces().substr(1, 1));
      _M_under.underlying().set_brackets({}, {});
      _M_under.underlying().set_separator(_Seps::_S_colon());
    } else if constexpr (format_kind<_Rg> == range_format::set)
      _M_under.set_brackets(_Seps::_S_braces().substr(0, 1), _Seps::_S_braces().substr(1, 1));
  }

  constexpr void set_separator(basic_string_view<_CharT> __sep) noexcept
    requires(format_kind<_Rg> == range_format::sequence)
  {
    _M_under.set_separator(__sep);
  }

  constexpr void set_brackets(basic_string_view<_CharT> __open, basic_string_view<_CharT> __close) noexcept
    requires(format_kind<_Rg> == range_format::sequence)
  {
    _M_under.set_brackets(__open, __close);
  }

  constexpr typename basic_format_parse_context<_CharT>::iterator parse(basic_format_parse_context<_CharT>& __pc) {
    auto __res = _M_under.parse(__pc);
    if constexpr (format_kind<_Rg> == range_format::debug_string) _M_under.set_debug_format();
    return __res;
  }

  template<typename _Out>
  constexpr typename basic_format_context<_Out, _CharT>::iterator
  format(__format::__maybe_const_range<_Rg, _CharT>& __rg, basic_format_context<_Out, _CharT>& __fc) const {
    if constexpr (_S_range_format_is_string) return _M_under._M_format_range(__rg, __fc);
    else return _M_under.format(__rg, __fc);
  }

private:
  using _Formatter_under =
    __conditional_t<_S_range_format_is_string, __format::__formatter_str<_CharT>, range_formatter<_Vt, _CharT>>;
  _Formatter_under _M_under;
};

#endif
#undef _GLIBCXX_WIDEN

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace std _GLIBCXX_VISIBILITY(default)
#endif
#pragma GCC diagnostic pop

int main(int argc, char** argv) {
  std::span<const char*> raw_args((const char**)argv + !!argc, (const char**)argv + argc);
  return std::format("too many arguments, unparsed: {::?}", raw_args).size();
}
