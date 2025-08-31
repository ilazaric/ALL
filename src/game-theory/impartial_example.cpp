#include <iomanip>

#include "impartial.hpp"

#include <ivl/logger>

#include <range/v3/view/concat.hpp>
// #include "/home/ilazaric/repos/range-v3/include/range/v3/view/concat.hpp"

struct Nim {
  std::size_t x;
  // this feels way too verbose
  auto moves() const {
    return std::views::iota(0ull, x) | std::views::transform([](std::size_t x) { return Nim{x}; });
  }
  auto operator<=>(const Nim&) const = default;
  bool operator==(const Nim&) const  = default;
};

template <>
struct std::hash<Nim> {
  std::size_t operator()(const Nim& o) const noexcept { return o.x; }
};

std::size_t hash_value(const Nim& o) {
  boost::hash<std::size_t> h;
  return h(o.x);
}

struct Queens;

struct EndSentinel {};

template <auto V>
struct IdV {};

template <typename T, typename>
struct IdC {
  using type = T;
};

template <typename T, typename U>
using Id = IdC<T, U>::type;

template <int dx, int dy>
struct Falling {
  using difference_type = std::ptrdiff_t;
  using value_type      = Queens;
  std::size_t         x, y;
  Falling             begin() const { return *this; }
  EndSentinel         end() const { return {}; }
  Id<Queens, IdV<dx>> operator*() const { return {x - dx, y - dy}; }
  Falling&            operator++() {
    x -= dx;
    y -= dy;
    return *this;
  }
  Falling operator++(int) { return {x - dx, y - dy}; }
  bool    operator==(const Falling&) const = default;
  bool    operator==(EndSentinel) const { return dx && !x || dy && !y; }
};

template <int dx, int dy>
inline constexpr bool std::ranges::enable_borrowed_range<Falling<dx, dy>> = true;
template <int dx, int dy>
inline constexpr bool ranges::enable_borrowed_range<Falling<dx, dy>> = true;

struct Queens {
  std::size_t x, y;
  auto moves() const { return ranges::views::concat(Falling<1, 0>{x, y}, Falling<0, 1>{x, y}, Falling<1, 1>{x, y}); }

  auto operator<=>(const Queens&) const = default;
  bool operator==(const Queens&) const  = default;
};

static_assert(std::ranges::borrowed_range<Falling<1, 1>>);

static_assert(ranges::input_or_output_iterator<Falling<1, 1>>);
static_assert(ranges::viewable_range<Falling<1, 1>>);
static_assert(ranges::input_range<Falling<1, 1>>);

static_assert(std::weakly_incrementable<Falling<1, 1>>);

std::size_t hash_value(const Queens& q) { return boost::hash<std::pair<std::size_t, std::size_t>>{}({q.x, q.y}); }

int main() {
  {
    ivl::gt::Grundifier<Nim> G;
    LOG(G(Nim{12}));
  }
  {
    ivl::gt::Grundifier<Queens> G;
    LOG(G(Queens{3, 5}));

    for (auto i : std::views::iota(0u, 10u)) {
      for (auto j : std::views::iota(0u, 10u))
        std::cout << std::setw(3) << G(Queens{i, j}) << " ";
      std::cout << std::endl;
    }
  }
}
