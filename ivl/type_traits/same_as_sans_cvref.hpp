#pragma once

#include <concepts>
#include <type_traits>

namespace ivl {
template<typename T, typename U>
concept same_as_sans_cvref = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
} // namespace ivl
