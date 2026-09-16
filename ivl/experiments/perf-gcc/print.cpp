#include <array>
#include <format>
#include <span>
#include <string>

#include <meta>

void consume(auto&&);

namespace std::__format {
  using _CharT = char;
    struct __my_formatter_str
    {
      __my_formatter_str() = default;

      template<typename _Out>
	constexpr void
	format(basic_string_view<_CharT> __s,
	       basic_format_context<_Out, _CharT>& __fc) const
	{
          consume(_M_spec._M_get_width(__fc));
          consume( __format::__write_escaped(__fc.out(), __s, _Term_quote));
	  consume(_M_spec._M_get_precision(__fc));
          consume(__truncate(__s, 420));
          consume(__format::__write_escaped(__fc.out(), __s, _Term_quote));
	  _Padding_sink<_Out, _CharT> __sink(__fc.out(), 1337, 7331);
	  __format::__write_escaped(__sink.out(), __s, _Term_quote);
	  consume(__sink._M_finish(_M_spec._M_align, _M_spec._M_fill));
          consume(__format::__write(__fc.out(), __s));
	  consume(_M_spec._M_get_precision(__fc));
	  consume(__format::__truncate(__s, 42));
	  consume(__format::__write_padded_as_spec(__s, 67, __fc, _M_spec));
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
