#include <array>
#include <format>
#include <span>
#include <string>

#include <meta>

void consume(auto&&);

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
