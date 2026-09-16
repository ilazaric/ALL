#include <array>
#include <format>
#include <span>
#include <string>

#include <meta>

void consume(auto&&);

consteval void describe(std::meta::info type, bool base = false, size_t indent = 0) {
  if (is_union_type(type)) throw;
  __builtin_constexpr_diag(
    32, "describe", std::string(indent, ' ') + std::string(base ? ": " : "") + display_string_of(type)
  );
  if (!is_class_type(type)) return;
  if (is_same_type(type, ^^std::__format::__formatter_ptr<char>)) return;
  if (is_same_type(type, ^^std::__format::__formatter_int<char>)) return;
  if (is_same_type(type, ^^std::__format::__formatter_str<char>)) return;
  if (is_same_type(type, ^^std::__format::__formatter_fp<char>)) return;
  if (base) indent += 2;
  auto ctx = std::meta::access_context::unchecked();
  for (auto b : bases_of(type, ctx)) describe(type_of(b), true, indent);
  for (auto b : nonstatic_data_members_of(type, ctx)) describe(type_of(b), false, indent + 2);
}

namespace std::__format {
  template<__char _CharT>
    struct __my_formatter_str
    {
      __my_formatter_str() = default;

      constexpr
      __my_formatter_str(_Spec<_CharT> __spec) noexcept
       : _M_spec(__spec)
      { }

      constexpr typename basic_format_parse_context<_CharT>::iterator
      parse(basic_format_parse_context<_CharT>& __pc)
      {
	auto __first = __pc.begin();
	const auto __last = __pc.end();
	_Spec<_CharT> __spec{};

	auto __finalize = [this, &__spec] {
	  _M_spec = __spec;
	};

	auto __finished = [&] {
	  if (__first == __last || *__first == '}')
	    {
	      __finalize();
	      return true;
	    }
	  return false;
	};

	if (__finished())
	  return __first;

	__first = __spec._M_parse_fill_and_align(__first, __last);
	if (__finished())
	  return __first;

	__first = __spec._M_parse_width(__first, __last, __pc);
	if (__finished())
	  return __first;

	__first = __spec._M_parse_precision(__first, __last, __pc);
	if (__finished())
	  return __first;

	if (*__first == 's')
	  {
	    __spec._M_type = _Pres_s;
	    ++__first;
	  }
#if __glibcxx_format_ranges // C++ >= 23 && HOSTED
	else if (*__first == '?')
	  {
	    __spec._M_debug = true;
	    ++__first;
	  }
#endif

	if (__finished())
	  return __first;

	__format::__failed_to_parse_format_spec();
      }

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

#if __glibcxx_format_ranges // C++ >= 23 && HOSTED
      template<ranges::input_range _Rg, typename _Out>
	requires same_as<remove_cvref_t<ranges::range_reference_t<_Rg>>, _CharT>
	constexpr _Out
	_M_format_range(_Rg&& __rg, basic_format_context<_Out, _CharT>& __fc) const
	{
	  using _Range = remove_reference_t<_Rg>;
	  using _String_view = basic_string_view<_CharT>;
	  if constexpr (ranges::contiguous_range<_Rg>)
	    {
	      _String_view __str(ranges::data(__rg),
				 size_t(ranges::distance(__rg)));
	      return format(__str, __fc);
	    }
	  else if constexpr (!is_const_v<_Range>
			        && __simply_formattable_range<_Range, _CharT>)
	    return _M_format_range<const _Range&>(__rg, __fc);
	  else if constexpr (!is_lvalue_reference_v<_Rg>)
	    return _M_format_range<_Range&>(__rg, __fc);
	  else
	    {
	      auto __handle_debug = [this, &__rg]<typename _NOut>(_NOut __nout)
		{
		  if (!_M_spec._M_debug)
		    return ranges::copy(__rg, std::move(__nout)).out;

		  _Escaping_sink<_NOut, _CharT>
		    __sink(std::move(__nout), _Term_quote);
		  ranges::copy(__rg, __sink.out());
		  return __sink._M_finish();
		};

	      const size_t __padwidth = _M_spec._M_get_width(__fc);
	      if (__padwidth == 0 && _M_spec._M_prec_kind == _WP_none)
		return __handle_debug(__fc.out());

	      _Padding_sink<_Out, _CharT>
		__sink(__fc.out(), __padwidth, _M_spec._M_get_precision(__fc));
	      __handle_debug(__sink.out());
	      return __sink._M_finish(_M_spec._M_align, _M_spec._M_fill);
	    }
	}

      constexpr void
      set_debug_format() noexcept
      { _M_spec._M_debug = true; }
#endif

    private:
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
