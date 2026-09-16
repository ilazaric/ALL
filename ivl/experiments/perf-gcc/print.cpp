#include <array>
#include <format>
#include <span>
#include <string>

#include <meta>

void consume(auto&&);

namespace std::__format {
  template<__char _CharT>
    struct __my_formatter_str
    {
      __my_formatter_str() = default;

      template<typename _Out>
	constexpr _Out
	format(basic_string_view<_CharT> __s,
	       basic_format_context<_Out, _CharT>& __fc) const
	{
	  if (_M_spec._M_debug)
	    return _M_format_escaped(__s, __fc);

	  if (_M_spec._M_width_kind == _WP_none
		&& _M_spec._M_prec_kind == _WP_none)
	    return __format::__write(__fc.out(), __s);

	  const size_t __maxwidth = _M_spec._M_get_precision(__fc);
	  const size_t __width = __format::__truncate(__s, __maxwidth);
	  return __format::__write_padded_as_spec(__s, __width, __fc, _M_spec);
	}

      template<typename _Out>
	constexpr _Out
	_M_format_escaped(basic_string_view<_CharT> __s,
			  basic_format_context<_Out, _CharT>& __fc) const
	{
	  const size_t __padwidth = _M_spec._M_get_width(__fc);
	  if (__padwidth == 0 && _M_spec._M_prec_kind == _WP_none)
	    return __format::__write_escaped(__fc.out(), __s, _Term_quote);

	  const size_t __maxwidth = _M_spec._M_get_precision(__fc);
	  const size_t __width = __truncate(__s, __maxwidth);
	  // N.B. Escaping only increases width
	  if (__padwidth <= __width && _M_spec._M_prec_kind == _WP_none)
	    return __format::__write_escaped(__fc.out(), __s, _Term_quote);

	  // N.B. [tab:format.type.string] defines '?' as
	  // Copies the escaped string ([format.string.escaped]) to the output,
	  // so precision seem to appy to escaped string.
	  _Padding_sink<_Out, _CharT> __sink(__fc.out(), __padwidth, __maxwidth);
	  __format::__write_escaped(__sink.out(), __s, _Term_quote);
	  return __sink._M_finish(_M_spec._M_align, _M_spec._M_fill);
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
  __format::__my_formatter_str<char> _M_f;
  _M_f.format(__arg, _M_fc);
}

template void foo_g<const char*>(const char*&);
template void foo_g<std::basic_string_view<char>>(std::basic_string_view<char>&);
} // namespace std::__format
