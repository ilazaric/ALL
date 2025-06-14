#pragma once

namespace ivl::aa {

  // TODO: probably add something along the lines of iterator categories

  // TODO: El is not just Ctx::element_type, maybe it should be
  //       this allows for transparent ctxs
  template <typename SemigroupContext, typename SemigroupElement>
  concept Semigroup =
    requires(SemigroupContext g, const SemigroupElement cel, SemigroupElement el) {
      g.multiply(cel, cel)->SemigroupElement;
      g.multiply_assign(el, cel)
        ->void; // TODO: this could return a reference, but that sounds a bit cringe
      // also associativity
      // also purity maybe?
      { cel == cel } -> bool;
      { cel != cel } -> bool;
      // also == is ! (!=)
      // also purity of eq-comparison
      // also alignment of eq-comparison with ops
      // - a == b, c == d --> a*b == c*d
    };

  template <typename MonoidContext, typename MonoidElement>
  concept Monoid = Semigroup<MonoidContext, MonoidElement> && requires {
    MonoidElement el {};
    // also ^ is identity
  };

  template <typename GroupContext, typename GroupElement>
  concept Group = Monoid<GroupContext, GroupElement> && requires(GroupContext       g,
                                                                 const GroupElement cel,
                                                                 GroupElement       el) {
    g.inverse(cel)->GroupElement;
    g.inverse_inplace(el)
      ->void; // TODO: we really dont need this, an interface could default onto inverse() + assign
    // ^ inverses wrt operation
  };

} // namespace ivl::aa
