#include "value_cell.hpp"

using namespace ivl::refl;

template <typename = decltype([] {})>
struct Primes {
private:
  using Cell = ValueCell<std::size_t>;

  static consteval bool initialize() {
    Cell::store(2);
    return true;
  }

  static_assert(initialize());

  static consteval bool is_prime(std::size_t n) {
    for (std::size_t d = 2; d * d <= n; ++d)
      if (n % d == 0) return false;
    return true;
  }

public:
  static consteval std::size_t next() {
    auto curr = Cell::load();
    while (not is_prime(curr))
      ++curr;
    Cell::store(curr + 1);
    return curr;
  }
};

using P = Primes<>;
static_assert(P::next() == 2);
static_assert(P::next() == 3);
static_assert(P::next() == 5);
static_assert(P::next() == 7);
static_assert(P::next() == 11);
static_assert(P::next() == 13);
static_assert(P::next() == 17);
static_assert(P::next() == 19);
