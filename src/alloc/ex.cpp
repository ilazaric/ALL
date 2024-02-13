#include <string_view>

template <unsigned N> struct fixed_string {
  char buf[N + 1]{};
  consteval fixed_string(char const *s) {
    for (unsigned i = 0; i != N; ++i)
      buf[i] = s[i];
    buf[N] = 0;
  }
  consteval operator std::string_view() const { return {buf, N}; }
};
template <unsigned N> fixed_string(char const (&)[N]) -> fixed_string<N - 1>;

template <fixed_string Str> struct storage {
  inline static constexpr std::string_view sv{Str};
  inline static constexpr bool hasA = sv.find('A') != std::string_view::npos;
};

int main() {
  using NS = storage<"ABC">;
  return NS::hasA;
}
