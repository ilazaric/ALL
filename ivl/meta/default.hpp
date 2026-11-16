#pragma once

// Metaprogramming utilities.

// TODO: <ivl/meta> should relate to <meta>, as in reflection
// ....: either kill this or move it somewhere

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace ivl::meta {
template<typename T, typename U>
concept is_unqualified = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
} // namespace ivl::meta
