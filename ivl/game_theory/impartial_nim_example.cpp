#include <ivl/logger>
#include "impartial"
#include <iomanip>
#include <ranges>

struct nim {
  size_t x;
  // this feels way too verbose
  auto moves() const {
    return std::views::iota(0ull, x) | std::views::transform([](size_t x) { return nim{x}; });
  }
  auto operator<=>(const nim&) const = default;
  bool operator==(const nim&) const = default;
};

template<>
struct std::hash<nim> {
  size_t operator()(const nim& o) const noexcept { return o.x; }
};

size_t hash_value(const nim& o) {
  boost::hash<size_t> h;
  return h(o.x);
}

int ivl_main() {
  ivl::game_theory::grundifier<nim> G;
  LOG(G(nim{12}));
  return 0;
}
