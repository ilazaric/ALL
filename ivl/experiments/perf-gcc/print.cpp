#include <array>
// #include <print>
#include <format>
#include <span>
#include <string>

#include <meta>

// void foo(std::format_args);

void consume(auto&&);

// namespace std::__format {
// template<typename _Visitor, typename _Ctx>
// inline constexpr decltype(auto) __my_visit_format_arg(_Visitor&& __vis, basic_format_arg<_Ctx> __arg) {
//   return __arg._M_visit(std::forward<_Visitor>(__vis));
// }
// } // namespace std::__format

// namespace std::__format {
// // Process a format string and format the arguments in the context.
// template<typename _Out, typename _CharT>
// class _my_Formatting_scanner : public _Scanner<_CharT> {
// public:
//   constexpr _my_Formatting_scanner(basic_format_context<_Out, _CharT>& __fc, basic_string_view<_CharT> __str)
//       : _Scanner<_CharT>(__str), _M_fc(__fc) {}

// private:
//   basic_format_context<_Out, _CharT>& _M_fc;

//   using iterator = typename _Scanner<_CharT>::iterator;

//   constexpr void _M_on_chars(iterator __last) override {}

//   constexpr void _M_format_arg(size_t __id) override {
//     using _Context = basic_format_context<_Out, _CharT>;
//     using handle = typename basic_format_arg<_Context>::handle;

//     auto lambda = [this](auto& __arg) {
//         using _Type = remove_reference_t<decltype(__arg)>;
//         using _Formatter = typename _Context::template formatter_type<_Type>;
//         if constexpr (is_same_v<_Type, monostate>) __format::__invalid_arg_id_in_format_string();
//         else if constexpr (is_same_v<_Type, handle>) __arg.format(this->_M_pc, this->_M_fc);
//         else if constexpr (is_default_constructible_v<_Formatter>) {
//       consteval { __builtin_constexpr_diag(32, "lambda_type", display_string_of(^^decltype(__arg))); }
//           _Formatter __f;
//           this->_M_pc.advance_to(__f.parse(this->_M_pc));
//           this->_M_fc.advance_to(__f.format(__arg, this->_M_fc));
//         } else static_assert(__format::__formattable_with<_Type, _Context>);
//     };

//     __format::__my_visit_format_arg(
//       lambda,
//       _M_fc.arg(__id)
//     );
//   }
// };
// } // namespace std::__format

// namespace std::__format {
// _Sink_iter<char>
// __my2_do_vformat_to(_Sink_iter<char> __out, string_view __fmt, __format_context<char>& __ctx) {
//   _my_Formatting_scanner<_Sink_iter<char>, char> __scanner(__ctx, __fmt);
//   consume(__scanner);
//   // __scanner._M_scan();
//   return __out;
// }
// }

namespace std::__format {
  format_context& _my_M_fc();
  format_parse_context& _my_M_pc();
  void foo_g(auto& __arg) {
    using _Context = format_context;
    using _Type = remove_reference_t<decltype(__arg)>;
    using _Formatter = typename _Context::template formatter_type<_Type>;
    _Formatter __f;
    _my_M_pc().advance_to(__f.parse(_my_M_pc()));
    _my_M_fc().advance_to(__f.format(__arg, _my_M_fc()));
  }
  
  template void foo_g<bool>(bool&);
  template void foo_g<char>(char&);
  template void foo_g<int>(int&);
  template void foo_g<unsigned int>(unsigned int&);
  template void foo_g<long long int>(long long int&);
  template void foo_g<long long unsigned int>(long long unsigned int&);
  template void foo_g<float>(float&);
  template void foo_g<double>(double&);
  template void foo_g<long double>(long double&);
  template void foo_g<__float128>(__float128&);
  template void foo_g<__bf16>(__bf16&);
  template void foo_g<_Float16>(_Float16&);
  template void foo_g<_Float32>(_Float32&);
  template void foo_g<_Float64>(_Float64&);
  template void foo_g<const char*>(const char*&);
  template void foo_g<std::basic_string_view<char>>(std::basic_string_view<char>&);
  template void foo_g<const void*>(const void*&);
  template void foo_g<__int128>(__int128&);
  template void foo_g<__int128 unsigned>(__int128 unsigned&);
}
