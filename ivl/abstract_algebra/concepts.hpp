#pragma once

#include <concepts>

namespace ivl::aa {

  // TODO: probably add something along the lines of iterator categories

  // TODO: El is not just Ctx::element_type, maybe it should be
  //       this allows for transparent ctxs
  template <typename SemigroupContext, typename SemigroupElement>
  concept Semigroup = requires(SemigroupContext g, const SemigroupElement cel, SemigroupElement el) {
    { g.multiply(cel, cel)} -> std::same_as<SemigroupElement>;
    { g.multiply_assign(el, cel) } -> std::same_as<void>; // TODO: this could return a reference, but that sounds a bit cringe
    // also associativity
    // also purity maybe?
    { cel == cel } -> std::same_as<bool>;
    { cel != cel } -> std::same_as<bool>;
    // also == is ! (!=)
    // also purity of eq-comparison
    // also alignment of eq-comparison with ops
    // - a == b, c == d --> a*b == c*d
  };

  template <typename MonoidContext, typename MonoidElement>
  concept Monoid = Semigroup<MonoidContext, MonoidElement> && requires {
    MonoidElement{};
    // also ^ is identity
  };

  template <typename GroupContext, typename GroupElement>
  concept Group =
    Monoid<GroupContext, GroupElement> && requires(GroupContext g, const GroupElement cel, GroupElement el) {
      { g.inverse(cel) } -> std::same_as<GroupElement>;
      { g.inverse_inplace(el) } -> std::same_as<void>; // TODO: we really dont need this, an interface could default onto inverse() + assign
      // ^ inverses wrt operation
    };

} // namespace ivl::aa
