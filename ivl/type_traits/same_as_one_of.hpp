#pragma once

#include <concepts>

namespace ivl {
template<typename T, typename... Ts>
concept same_as_one_of = (std::same_as<T, Ts> || ...);
} // namespace ivl
