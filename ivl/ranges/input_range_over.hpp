#pragma once

#include <concepts>
#include <ranges>
#include <type_traits>

namespace ivl::ranges {
// Should answer "can i range-for over it, with type `V`?"
// Inspired by `reflection_range`.
template<typename Rg, typename V>
concept input_range_over = std::ranges::input_range<Rg> && std::same_as<std::ranges::range_value_t<Rg>, V> &&
                           std::same_as<std::remove_cvref_t<std::ranges::range_reference_t<Rg>>, V>;
} // namespace ivl::ranges
