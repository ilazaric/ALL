// via define_aggregate()

#include <meta>
#include <print>
#include <string>
#include <string_view>
#include <vector>

template<auto>
struct storage {};

consteval void store(std::meta::info type, std::meta::info value) {
  auto wrapped = std::meta::reflect_constant(value);
  auto stored = substitute((^^storage), {wrapped});
  auto spec = data_member_spec(stored, {.name = "storage"});
  define_aggregate(type, {spec});
}

consteval std::meta::info load(std::meta::info type) {
  auto stored = nonstatic_data_members_of(type, std::meta::access_context::unchecked())[0];
  auto wrapped = template_arguments_of(type_of(stored))[0];
  auto value = extract<std::meta::info>(wrapped);
  return value;
}

void bla() {
  struct S;
  consteval {
    long x = 1;
    static constexpr auto r = ^^x;
    consteval {
      store(^^S, r);
    }
    consteval {
      [:load(^^S):] = 2;
    }
    __builtin_constexpr_diag(32, "", std::format("{}", x));
  }
  // std::println("{} -- {} {}", __func__, (const void*)&[:load(^^S):], [:load(^^S):]);
  // [:load(^^S):] = {};
  // std::println("{} -- {} {}", __func__, (const void*)&[:load(^^S):], [:load(^^S):]);
}

int main() {
  bla();
}
