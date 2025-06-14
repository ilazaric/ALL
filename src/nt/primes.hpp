#pragma once

namespace ivl::nt {

  // the slow stupid form
  bool is_prime(std::uint64_t n) {
    for (std::uint64_t p = 2; p * p <= n; ++p)
      if (n % p == 0)
        return false;
    return true;
  }

} // namespace ivl::nt
