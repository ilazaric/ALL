#pragma once

// Basically `std::type_identity`.
namespace ivl {
template<typename T>
struct tag_type {
  using type = T;
};
} // namespace ivl
