#include <experimental/meta>

template<typename>
struct S {
  inline static int value = 42;
};

template<auto&>
struct T {};

T<S<int>::value> t;

int& ref = []->auto&{
  auto a = ^S;
  auto b = substitute(a, {^float});
  auto c = static_data_members_of(b);
  return value_of<int&>(c[0]);
 }();
