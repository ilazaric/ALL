#include <ivl/logger/logger>
using namespace ivl::logger::default_logger;

#include <ivl/literals/ints>
using namespace ivl::literals::ints_exact;

#include <ivl/nt/primes>

int main() {
  constexpr auto A     = 1_u64 << 15;
  constexpr auto Mod   = 1'000'000'007_u64;
  int            found = 0;
  for (std::uint64_t curr = A + 1; found < 10; curr += A)
    if (curr > Mod && ivl::nt::is_prime(curr)) {
      LOG(curr);
      ++found;
    }
}
