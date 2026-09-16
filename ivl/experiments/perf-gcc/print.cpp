#include <array>
// #include <print>
#include <format>
#include <span>
#include <string>

// void foo(std::format_args);

void consume(auto&&);

namespace std::__format {
template<typename _Visitor, typename _Ctx>
inline constexpr decltype(auto) __my_visit_format_arg(_Visitor&& __vis, basic_format_arg<_Ctx> __arg) {
  return __arg._M_visit(std::forward<_Visitor>(__vis));
}
} // namespace std::__format

namespace std::__format {
// Process a format string and format the arguments in the context.
template<typename _Out, typename _CharT>
class _my_Formatting_scanner : public _Scanner<_CharT> {
public:
  constexpr _my_Formatting_scanner(basic_format_context<_Out, _CharT>& __fc, basic_string_view<_CharT> __str)
      : _Scanner<_CharT>(__str), _M_fc(__fc) {}

private:
  basic_format_context<_Out, _CharT>& _M_fc;

  using iterator = typename _Scanner<_CharT>::iterator;

  constexpr void _M_on_chars(iterator __last) override {}

  constexpr void _M_format_arg(size_t __id) override {
    using _Context = basic_format_context<_Out, _CharT>;
    using handle = typename basic_format_arg<_Context>::handle;

    __format::__my_visit_format_arg(
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
} // namespace std::__format

namespace std::__format {
template<typename _CharT>
constexpr _Sink_iter<_CharT>
__my_do_vformat_to(_Sink_iter<_CharT> __out, basic_string_view<_CharT> __fmt, __format_context<_CharT>& __ctx) {
  _my_Formatting_scanner<_Sink_iter<_CharT>, _CharT> __scanner(__ctx, __fmt);
  consume(__scanner);
  // __scanner._M_scan();
  return __out;
}
} // namespace std::__format

namespace std::__format {
template<typename _Out, typename _CharT, typename _Context>
[[gnu::noinline]]
inline constexpr _Out
__do_vformat_to_no_locale(_Out __out, basic_string_view<_CharT> __fmt, const basic_format_args<_Context>& __args) {
  static_assert(is_same_v<_Out, _Sink_iter<_CharT>>);
  auto __ctx = _Context(__args, __out);
  // consume(__ctx);
  // return std::move(__out);
  return __format::__my_do_vformat_to(__out, __fmt, __ctx);
}
} // namespace std::__format

template<typename _Out>
_Out my_vformat_to(_Out __out, std::string_view __fmt, std::format_args __args) {
  return std::__format::__do_vformat_to_no_locale(std::move(__out), __fmt, __args);
}

std::string my_vformat(std::string_view __fmt, std::format_args __args) {
  std::__format::_Str_sink<char> __buf;
  my_vformat_to(__buf.out(), __fmt, __args);
  return std::move(__buf).get();
}

// int main() {
//   std::array<const char*, 4> a{};
//   // return std::format("hello world big array: {::?}", std::span<const char*>(a)).size();
//   auto s = std::span<const char*>(a);
//   // return std::vformat("hello world big array: {::?}", std::make_format_args(s)).size();
//   // std::__format::_Str_sink<char> __buf;
//   // foo(std::make_format_args(s));
//   // return my_vformat("hello world big array: {::?}", std::make_format_args(s)).size();
// }
