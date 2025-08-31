#include <limits>
#include <random>
#include <vector>

#include <ivl/nt/multimint.hpp>
#include <ivl/nt/util.hpp>

#include <ivl/io/stlutils.hpp>
using namespace ivl::io;

#include <ivl/logger/logger.hpp>
using namespace ivl::logger::default_logger;

std::mt19937 gen(101);

void test() {
  std::vector<std::uint32_t> primes;
  for (auto idx : std::views::iota(2_u32)) {
    if (primes.size() == 1000) break;
    if (ivl::nt::is_prime(idx)) primes.push_back(idx);
  }

  std::vector<std::pair<std::uint32_t, std::uint32_t>> data;
  std::uniform_int_distribution<std::uint32_t>         dist(0_u32, std::numeric_limits<std::uint32_t>::max());
  for (auto idx : std::views::iota(0, 100)) {
    data.emplace_back(dist(gen), dist(gen));
  }

  std::vector<std::uint32_t> rems(primes.size(), 0);
  for (auto [x, y] : data)
    for (auto idx : std::views::iota(0_u32, primes.size()))
      rems[idx] = (std::uint32_t)(((std::uint64_t)rems[idx] * x % primes[idx] + y) % primes[idx]);

  using RT = ivl::nt::MultiMint<1'000'000'007>;
  RT acc;
  for (auto [x, y] : data)
    acc = acc * RT{x} + RT{y};

  ivl::nt::GarnerTable gt(primes);
  auto                 gv   = gt.convert_representation(rems.data());
  auto                 accg = gt.template apply<RT>(gv);
  // auto g = ivl::nt::garner(rems.data(), gs);
  // auto accg = ivl::nt::garner_apply<RT>(g, gs);
  if (acc[0] == accg[0]) return;

  LOG(acc[0], accg[0]);
  LOG(primes);
  LOG(gt.mods);
  LOG(gt.invs);
  LOG(data);
  LOG(rems);
  exit(-1);
}

int main() {
  for (auto rep : std::views::iota(0, 100))
    test();

  return 0;
}
