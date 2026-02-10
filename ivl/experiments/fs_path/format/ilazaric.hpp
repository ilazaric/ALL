#include <filesystem>
#include <format>

namespace std {
namespace __format
{
  template<__char _CharT>
    struct __formatter_fs_path
    {
      __formatter_fs_path() = default;

      constexpr
      __formatter_fs_path(_Spec<_CharT> __spec) noexcept
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

	if (*__first == '?')
	  {
	    __spec._M_debug = true;
	    ++__first;
	  }
	if (__finished())
	  return __first;

	if (*__first == 'g')
	  {
	    __spec._M_type = _Pres_g;
	    ++__first;
	  }
	if (__finished())
	  return __first;

	__format::__failed_to_parse_format_spec();
      }

      template<typename _Out>
	_Out
	format(const filesystem::path& __p,
	       basic_format_context<_Out, _CharT>& __fc) const
      {
	using _Value_type = filesystem::path::value_type;

	auto __write_str = [&](const auto& __s)
	{
	  auto __spec = _M_spec;
	  // 'g' should not be passed along.
	  __spec._M_type = _Pres_none;
	  __formatter_str<_CharT> __fmt_str(__spec);
	  return __fmt_str.format(__s, __fc);
	};

	auto __write_transcode = [&](const auto& __s)
	{
	  if constexpr (is_same_v<_CharT, _Value_type>)
	    {
	      return __write_str(__s);
	    }
	  else
	    {
	      basic_string<_CharT> __out;
	      using _View = basic_string_view<_Value_type>;
	      __out.assign_range(__unicode::_Utf_view<_CharT, _View>(__s));
	      return __write_str(__out);
	    }
	};

	if (_M_spec._M_type == _Pres_g)
	  return __write_transcode(__p.generic_string<_Value_type>());
	else
	  return __write_transcode(__p.native());
      }

      constexpr void
      set_debug_format() noexcept
      { _M_spec._M_debug = true; }

    private:
      _Spec<_CharT> _M_spec{};
    };
} // namespace __format

  template<__format::__char _CharT>
    struct formatter<filesystem::path, _CharT>
    {
      formatter() = default;

      [[__gnu__::__always_inline__]]
      constexpr typename basic_format_parse_context<_CharT>::iterator
      parse(basic_format_parse_context<_CharT>& __pc)
      { return _M_f.parse(__pc); }

      template<typename _Out>
	typename basic_format_context<_Out, _CharT>::iterator
	format(const filesystem::path& __p,
	       basic_format_context<_Out, _CharT>& __fc) const
	{ return _M_f.format(__p, __fc); }

      constexpr void set_debug_format() noexcept { _M_f.set_debug_format(); }

    private:
      __format::__formatter_fs_path<_CharT> _M_f;
    };
} // namespace std
