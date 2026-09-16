#include <array>
#include <format>
#include <span>
#include <string>

#include <meta>

void consume(auto&&);

namespace std::__format {

  template<typename _CharT, typename _Out>
    constexpr _Out
    __my_write_escaped_unicode_part(_Out __out, basic_string_view<_CharT>& __str,
				 bool& __prev_esc, _Term_char __term)
    {
      using _Str_view = basic_string_view<_CharT>;
      using _Esc = _Escapes<_CharT>;

      static constexpr char32_t __replace = U'\uFFFD';
      static constexpr _Str_view __replace_rep = []
	{
	  // N.B. "\uFFFD" is ill-formed if encoding is not unicode.
	  if constexpr (is_same_v<char, _CharT>)
	    return "\xEF\xBF\xBD";
	  else
	    return L"\xFFFD";
	}();

      __unicode::_Utf_view<char32_t, _Str_view> __v(std::move(__str));
      __str = {};

      auto __first = __v.begin();
      auto const __last = __v.end();
      while (__first != __last)
	{
	  bool __esc_ascii = false;
	  bool __esc_unicode = false;
	  bool __esc_replace = false;
	  auto __should_escape = [&](auto const& __it)
	    {
	      if (*__it <= 0x7f)
		return __esc_ascii
			 = __format::__should_escape_ascii(*__it.base(), __term);
	      if (__format::__should_escape_unicode(*__it, __prev_esc))
		return __esc_unicode = true;
	      if (*__it == __replace)
		{
		  _Str_view __units(__it.base(), __it._M_units());
		  return __esc_replace = (__units != __replace_rep);
		}
	      return false;
	    };

	  auto __print = __first;
	  while (__print != __last && !__should_escape(__print))
	    {
	      __prev_esc = false;
	      ++__print;
	    }

	  if (__print != __first)
	    __out = __format::__write(__out, _Str_view(__first.base(), __print.base()));

	  if (__print == __last)
	    return __out;

	  __first = __print;
	  if (__esc_ascii)
	    __out = __format::__write_escaped_char(__out, *__first.base());
	  else if (__esc_unicode)
	    __out = __format::__write_escape_seq(__out, *__first, _Esc::_S_u());
	  // __esc_replace
	  else if (_Str_view __units(__first.base(), __first._M_units());
		   __units.end() != __last.base())
	    __out = __format::__write_escape_seqs(__out, __units);
	  else
	    {
	      __str = __units;
	      return __out;
	    }

	  __prev_esc = true;
	  ++__first;
	}

      return __out;
    }

  template<typename _CharT, typename _Out>
    constexpr _Out
    __my_write_escaped_unicode(_Out __out, basic_string_view<_CharT> __str,
			    _Term_char __term)
    {
      bool __prev_escape = true;
      __out = __format::__my_write_escaped_unicode_part(__out, __str,
						     __prev_escape, __term);
      __out = __format::__write_escape_seqs(__out, __str);
      return __out;
    }
}

namespace std::__format {
  using _CharT = char;

  template<typename _Out>
    constexpr _Out
    __my_write_escaped(_Out __out,  basic_string_view<_CharT> __str, _Term_char __term)
    {
      __out = __format::__write(__out, _Escapes<_CharT>::_S_term(__term));
      static_assert(__unicode::__literal_encoding_is_unicode<_CharT>());
      __out = __format::__my_write_escaped_unicode(__out, __str, __term);
      return __format::__write(__out, _Escapes<_CharT>::_S_term(__term));
    }

  struct __my_formatter_str
    {
      __my_formatter_str() = default;

      template<typename _Out>
	constexpr void
	format(basic_string_view<_CharT> __s,
	       basic_format_context<_Out, _CharT>& __fc) const
	{
          consume(_M_spec._M_get_width(__fc));
          consume( __format::__my_write_escaped(__fc.out(), __s, _Term_quote));
	  _Padding_sink<_Out, _CharT> __sink(__fc.out(), 1337, 7331);
	  consume(__format::__my_write_escaped(__sink.out(), __s, _Term_quote));
	  // consume(__sink._M_finish(_M_spec._M_align, _M_spec._M_fill));
          // consume(__format::__write(__fc.out(), __s));
	  // consume(_M_spec._M_get_precision(__fc));
	  // consume(__format::__truncate(__s, 42));
	  // consume(__format::__write_padded_as_spec(__s, 67, __fc, _M_spec));
	}

      _Spec<_CharT> _M_spec{};
    };
}

namespace std::__format {
format_context& _my_M_fc();
void foo_g(auto& __arg) {
  using _Context = format_context;
  using _Type = remove_reference_t<decltype(__arg)>;
  auto& _M_fc = _my_M_fc();
  __format::__my_formatter_str _M_f;
  _M_f.format(__arg, _M_fc);
}

template void foo_g<const char*>(const char*&);
template void foo_g<std::basic_string_view<char>>(std::basic_string_view<char>&);
} // namespace std::__format
