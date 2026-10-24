#pragma once

#include <ivl/utility/forward>
#include <cstddef>
#include <optional>
#include <vector>

namespace ivl::algorithm {
// kinda like optional<T&>
template<typename T>
struct monotonic_vector_update_optional {
  std::vector<T>& v;
  size_t idx;
  bool set;

  monotonic_vector_update_optional(const monotonic_vector_update_optional&) = delete;
  monotonic_vector_update_optional(monotonic_vector_update_optional&&) = delete;
  monotonic_vector_update_optional& operator=(const monotonic_vector_update_optional&) = delete;
  monotonic_vector_update_optional& operator=(monotonic_vector_update_optional&&) = delete;

  void emplace(auto&&... args) {
    if (v.size() == idx) v.emplace_back(FWD(args)...);
    else v[idx] = T(FWD(args)...);
    set = true;
  }

  void reset() { set = false; }
  operator bool() { return set; }
  T& operator*() { return v[idx]; }
  T* operator->() { return &v[idx]; }
};
} // namespace ivl::algorithm
