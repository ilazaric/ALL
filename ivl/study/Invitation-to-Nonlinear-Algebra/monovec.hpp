#pragma once

#include <ivl/algorithm/monotonic_vector_update>
#include <ivl/utility/forward>
#include <flat_map>
#include <utility>

template<class Key, class T, class Compare = std::less<Key>>
struct flat_map : std::flat_map {
  using std::flat_map::flat_map;

  void merge(const flat_map& o, auto&& combine_or_erase, auto&& new_insert) noexcept {
    auto [keys, values] = std::move(*this).extract();
    ivl::algorithm::monotonic_vector_update(vector_tie(keys, values), vector_tie(o.keys(), o.values()), );
    replace(std::move(keys), std::move(values));
  }
};
