#include <ivl/logger>
#include "impartial"
#include <iomanip>
#include <ranges>

struct queens {
  size_t x, y;
  auto moves() const;
  auto operator<=>(const queens&) const = default;
  bool operator==(const queens&) const = default;
};

struct end_sentinel {};

template<int dx, int dy>
struct falling {
  using difference_type = std::ptrdiff_t;
  using value_type = queens;
  size_t x, y;
  falling begin() const { return *this; }
  end_sentinel end() const { return {}; }
  queens operator*() const { return {x - dx, y - dy}; }
  falling& operator++() {
    x -= dx;
    y -= dy;
    return *this;
  }
  falling operator++(int) { return {x - dx, y - dy}; }
  bool operator==(const falling&) const = default;
  bool operator==(end_sentinel) const { return dx && !x || dy && !y; }
};

template<int dx, int dy>
inline constexpr bool std::ranges::enable_borrowed_range<falling<dx, dy>> = true;

static_assert(std::ranges::borrowed_range<falling<1, 1>>);

static_assert(std::input_or_output_iterator<falling<1, 1>>);
static_assert(std::ranges::viewable_range<falling<1, 1>>);
static_assert(std::ranges::input_range<falling<1, 1>>);

static_assert(std::weakly_incrementable<falling<1, 1>>);

auto queens::moves() const { return std::views::concat(falling<1, 0>{x, y}, falling<0, 1>{x, y}, falling<1, 1>{x, y}); }

size_t hash_value(const queens& q) { return boost::hash<std::pair<size_t, size_t>>{}({q.x, q.y}); }

int ivl_main() {
  ivl::game_theory::grundifier<queens> G;
  LOG(G(queens{3, 5}));
  for (auto i : std::views::iota(0u, 10u)) {
    for (auto j : std::views::iota(0u, 10u)) std::cout << std::setw(3) << G(queens{i, j}) << " ";
    std::cout << std::endl;
  }
  return 0;
}
