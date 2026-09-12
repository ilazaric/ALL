#include <meta>
#include <print>
#include <vector>

template<typename T>
struct A {
  friend consteval auto fn(A);
};

template<typename T, auto V>
struct B {
  friend consteval auto fn(A<T>) { return V; }
};

void access_out_of_scope_static() {
  if (0) {
    static std::vector<std::string> x = {"hello", " ", "world", "\n"};
    consteval { size_of(substitute((^^B), {(^^int), std::meta::reflect_constant(^^x)})); }
  }
  std::println("{} -- {}", __func__, (const void*)&[:fn(A<int>{}):]);
}

void access_really_out_of_scope_static() { //
  std::println("{} -- {}", __func__, (const void*)&[:fn(A<int>{}):]);
}

void access_out_of_scope_locals() {
  if (0) {
    int x = 1;
    static constexpr auto r = ^^x;
    consteval { size_of(substitute((^^B), {(^^char), std::meta::reflect_constant(r)})); }
  }
  if (0) {
    int x = 2;
    static constexpr auto r = ^^x;
    consteval { size_of(substitute((^^B), {(^^short), std::meta::reflect_constant(r)})); }
  }
  std::println("{} -- {} {}", __func__, (const void*)&[:fn(A<char>{}):], [:fn(A<char>{}):]);
  std::println("{} -- {} {}", __func__, (const void*)&[:fn(A<short>{}):], [:fn(A<short>{}):]);
  static_cast<volatile int&>([:fn(A<char>{}):]) = 0;
  static_cast<volatile int&>([:fn(A<short>{}):]) = 0;
  std::println("{} -- {} {}", __func__, (const void*)&[:fn(A<char>{}):], [:fn(A<char>{}):]);
  std::println("{} -- {} {}", __func__, (const void*)&[:fn(A<short>{}):], [:fn(A<short>{}):]);
}

// void access_really_out_of_scope_locals() {
//   std::println("{} -- {} {}", __func__, (const void*)&[:fn(A<char>{}):], [:fn(A<char>{}):]);
//   std::println("{} -- {} {}", __func__, (const void*)&[:fn(A<short>{}):], [:fn(A<short>{}):]);
// }

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
  if (0) {
    int x = 1;
    static constexpr auto r = ^^x;
    consteval { store(^^S, r); }
  }
  std::println("{} -- {} {}", __func__, (const void*)&[:load(^^S):], [:load(^^S):]);
}

int main() {
  access_out_of_scope_static();
  access_really_out_of_scope_static();
  access_out_of_scope_locals();
  bla();
  // access_really_out_of_scope_locals();
  // auto vec = use();
  // std::println("length: {}", vec.size());
  // std::vector<std::string> empty;
  // std::println("empty: {:?}", std::string_view((const char*)&empty, (const char*)(&empty + 1)));
  // std::println("vec  : {:?}", std::string_view((const char*)&vec, (const char*)(&vec + 1)));
}
