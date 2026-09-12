#include <meta>

template<typename>
struct storage {
  static inline int value = 2;
};

consteval int* find_value(std::meta::info type) {
  for (auto member : members_of(type, std::meta::access_context::unchecked())) {
    if (!has_identifier(member)) continue;
    if (identifier_of(member) != "value") continue;
    return &extract<int&>(member);
  }
  throw;
}

template<typename T>
consteval int* find_value2_impl() {
  return &T::value;
}

consteval int* find_value2(std::meta::info type) {
  return extract<int*(*)()>(substitute(^^find_value2_impl, {type}))();
}

int main() { //
  return *find_value(^^storage<int>);
}
