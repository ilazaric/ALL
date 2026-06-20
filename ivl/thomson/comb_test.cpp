#include <ivl/logger>
#include "comb"
#include <map>
#include <meta>
#include <print>

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

std::size_t concat(std::size_t n, std::size_t m1, std::size_t enc1, std::size_t m2, std::size_t enc2) {
  contract_assert(enc1 < comb_count_smart(n, m1));
  contract_assert(enc2 < comb_count_smart(n, m2));
  auto dec = decode_first(n, m1, enc1);
  dec.insert_range(dec.end(), decode_first(n, m2, enc2));
  return encode_weird2(n, m1 + m2, dec);
}

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
      if constexpr (has_identifier(refl) && identifier_of(refl).starts_with("decode_")) {
        std::cerr << "validating " << identifier_of(refl) << " ...\n";
        for (std::size_t i = 0; i < combs.size(); ++i) {
          0; // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=125904
          contract_assert(combs[i] == [:refl:](n, m, i));
        }
      }
    }
  }

  {
    std::size_t n = 4;
    std::size_t m1 = 3;
    std::size_t m2 = 4;
    LOG(n, m1, comb_count_smart(n, m1));
    LOG(n, m2, comb_count_smart(n, m2));
    LOG(n, m1 + m2, comb_count_smart(n, m1 + m2));
    std::string sep;
    std::print(" \\ ");
    sep += "---";
    for (std::size_t e1 = 0; e1 < comb_count_smart(n, m1); ++e1) {
      std::print(" | {: ^3}", e1);
      sep += "-+----";
    }
    std::println();
    std::map<std::size_t, std::size_t> counts;
    for (std::size_t e2 = 0; e2 < comb_count_smart(n, m2); ++e2) {
      std::println("{}", sep);
      std::print("{: ^3}", e2);
      for (std::size_t e1 = 0; e1 < comb_count_smart(n, m1); ++e1) {
        std::print(" | {: ^3}", concat(n, m1, e1, m2, e2));
        if (e1) ++counts[concat(n, m1, e1, m2, e2) - concat(n, m1, e1 - 1, m2, e2)];
        if (e2) ++counts[concat(n, m1, e1, m2, e2) - concat(n, m1, e1, m2, e2 - 1)];
      }
      std::println();
    }
    for (auto&& [k, v] : counts) LOG(k, v);
  }

  {
    std::size_t n = 4;
    std::size_t m1 = 3;
    std::size_t m2 = 4;
    LOG(n, m1, comb_count_smart(n, m1));
    LOG(n, m2, comb_count_smart(n, m2));
    LOG(n, m1 + m2, comb_count_smart(n, m1 + m2));
    std::size_t e1 = 12;
    LOG(e1);
    std::println("e2  | mix | delta");
    std::size_t last = 12;
    std::map<std::size_t, std::size_t> counts;
    for (std::size_t e2 = 0; e2 < comb_count_smart(n, m2); ++e2) {
      std::size_t curr = concat(n, m1, e1, m2, e2);
      std::println("{: ^3} | {: ^3} | {: ^3}", e2, curr, curr - last);
      if (e2) ++counts[curr - last];
      last = curr;
    }
    for (auto&& [k, v] : counts) LOG(k, v);
  }

  return 0;
}
