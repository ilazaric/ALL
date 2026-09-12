#include <meta>
#include <string_view>

template<typename T>
struct implicit_parsed_storage {
  // TOOD: gcc bug here?
  static inline int value = 2;
};

consteval int* find_value(std::meta::info storage) {
  for (auto member : members_of(storage, std::meta::access_context::unchecked())) {
    if (!has_identifier(member)) continue;
    if (identifier_of(member) != "value") continue;
    return &extract<int&>(member);
  }
  contract_assert(false);
}

template<typename T>
struct implicit_name {
    int* value_ptr;

  consteval implicit_name() {
    value_ptr = find_value(^^implicit_parsed_storage<bool>);
  }
};

// IVL add_compiler_flags("-Wno-non-template-friend -Wsfinae-incomplete=0")

int main() {
  return *implicit_name<bool>().value_ptr;
  // return implicit_flag("foo");
}
