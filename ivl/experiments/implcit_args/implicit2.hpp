#include <meta>
#include <string_view>

template<typename T>
struct implicit_parsed_storage {
  // TOOD: gcc bug here?
  static inline int value = std::nullopt;
};

template<typename T>
struct implicit_name {
    int* value_ptr;

  inline static consteval int* find_value(std::meta::info storage) {
    for (auto member : members_of(storage, std::meta::access_context::unchecked())) {
      if (!has_identifier(member)) continue;
      if (identifier_of(member) != "value") continue;
      return &extract<int&>(member);
    }
    contract_assert(false);
  }

  consteval implicit_name() {
    value_ptr = find_value(^^implicit_parsed_storage<bool>);
  }
};
