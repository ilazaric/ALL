#include "memory_cell.hpp"

using namespace ivl::refl;

template <typename = decltype([] {})>
struct Primes {
private:
  using Cell = MemoryCell<>;

  static consteval bool initialize() {
    Cell::store(std::meta::reflect_value((std::size_t)2));
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
    std::size_t curr = std::meta::extract<std::size_t>(Cell::load());
    while (not is_prime(curr))
      ++curr;
    Cell::store(std::meta::reflect_value(curr + 1));
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
