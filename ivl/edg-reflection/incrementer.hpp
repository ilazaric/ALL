#pragma once

#include <bit>
#include <cstdint>
#include <experimental/meta>

namespace ivl::refl {

  // represents an incrementable number
  // API: advance(), get()
  // advance(): increments
  // get(): returns number of advances that happened
  //
  // recommended use:
  // 1) using Inc = Incrementer<>;
  // - Inc::advance();
  // - Inc::get();
  // 2) static constexpr Incrementer<> inc;
  // - inc.advance();
  // - inc.get();
  template <typename = decltype([] {})>
  struct Incrementer {
  private:
    template <std::size_t, std::size_t>
    struct Data;

    static consteval std::meta::info get(std::size_t a, std::size_t b) {
      auto datai  = ^Data;
      auto ai     = std::meta::reflect_value(a);
      auto bi     = std::meta::reflect_value(b);
      auto classi = std::meta::substitute(datai, {ai, bi});
      return classi;
    }

    static consteval bool test(std::size_t a, std::size_t b) { return not std::meta::is_incomplete_type(get(a, b)); }

    static consteval std::size_t find_first_unset() {
      std::size_t exp = 0;
      while (test(0, exp))
        ++exp;
      std::size_t res = 0;
      for (; exp != (std::size_t)-1; --exp)
        if (test(res, exp)) res += (1ull << exp);
      return res;
    }

  public:
    static consteval std::size_t get() { return find_first_unset(); }

    static consteval void advance() {
      std::size_t idx    = find_first_unset() + 1;
      std::size_t exp    = std::countr_zero(idx);
      std::size_t prev   = idx ^ (1ull << exp);
      auto        classi = get(prev, exp);
      std::meta::define_class(classi, {});
    }
  };

  // for now this broken, look at edg-bugs/CTAD.cpp
  // template<typename T = decltype([]{})>
  // Incrementer() -> Incrementer<T>;

} // namespace ivl::refl
