#include "concepts"
#include "polynomial"
#include "rings"
#include "synth_field"

static_assert(gcd_ring<Z>);
static_assert(field<Q>);
static_assert(field<Ratio<Z>>);
static_assert(ring<Poly<Z>>);
static_assert(ring<Poly<Q>>);

// IVL add_compiler_flags_tail("-lgmpxx -lgmp")

int ivl_main() {
  {
    Poly<Z> p{{1, 1}};
    ivl::fmt::println("{}", p * p * p);
  }
  {
    Poly<Z> p{{-1, 1}};
    ivl::fmt::println("{:z}", p * p * p);
  }
  {
    Poly<Q> p{{-3_mpq / 2, 2}};
    ivl::fmt::println("{}", p * p * p);
  }
  {
    Poly<Ratio<Z>> p{{{-3, 2}, {2, 1}}};
    ivl::fmt::println("{}", p * p * p);
  }
  return 0;
}
