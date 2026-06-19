#include <ivl/logger>
#include "comb"
#include <meta>

void gen_combs_r(
  std::size_t n, std::size_t m, std::size_t idx, std::vector<std::size_t>& curr,
  std::vector<std::vector<std::size_t>>& ret
) {
  if (idx == m) {
    ret.push_back(curr);
    return;
  }
  for (std::size_t i = idx ? curr[idx - 1] : 0; i <= n; ++i) {
    curr[idx] = i;
    gen_combs_r(n, m, idx + 1, curr, ret);
  }
}

std::vector<std::vector<std::size_t>> gen_combs(std::size_t n, std::size_t m) {
  std::vector<std::vector<std::size_t>> ret;
  std::vector<std::size_t> curr(m, 0);
  gen_combs_r(n, m, 0, curr, ret);
  return ret;
}

std::size_t comb_count_stupid(std::size_t n, std::size_t m) { return gen_combs(n, m).size(); }
std::size_t comb_count_smart(std::size_t n, std::size_t m) { return choose(n + m, n); }

int ivl_main() {
  contract_assert(choose(4, 2) == 6);
  contract_assert(choose(6, 2) == 15);
  contract_assert(choose(6, 3) == 20);
  for (std::size_t n = 0; n <= 4; ++n)
    for (std::size_t m = 0; m <= 6; ++m) contract_assert(comb_count_stupid(n, m) == comb_count_smart(n, m));

  {
    std::size_t n = 5;
    std::size_t m = 8;
    auto combs = gen_combs(n, m);
    LOG(combs.size());

    template for (constexpr auto refl : define_static_array(members_of(^^::, std::meta::access_context::unchecked()))) {
      if constexpr (has_identifier(refl) && identifier_of(refl).starts_with("encode_")) {
        std::cerr << "validating " << identifier_of(refl) << " ...\n";
        for (std::size_t i = 0; i < combs.size(); ++i) {
          0; // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=125904
          contract_assert(i == [:refl:](n, m, combs[i]));
        }
      }
    }

    // for (std::size_t i = 0; i < combs.size(); ++i) {
    //   contract_assert(i == encode_stupid(n, m, combs[i]));
    //   contract_assert(i == encode_smart(n, m, combs[i]));
    //   contract_assert(i == encode_smart2(n, m, combs[i]));
    //   contract_assert(i == encode_smart3(n, m, combs[i]));
    //   contract_assert(i == encode_smart4(n, m, combs[i]));
    //   contract_assert(i == encode_smart5(n, m, combs[i]));
    // }
  }

  return 0;
}
