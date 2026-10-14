#include <ivl/command_line_argument_parsing/implicit_exposed>
#include <ivl/format>
#include <algorithm>
#include <ranges>
#include <set>
#include <vector>

using perm = std::vector<size_t>;

perm id(size_t n) { return perm(std::from_range, std::views::iota(0UZ, n)); }

perm cycle(size_t n) {
  perm p(n);
  for (size_t i = 0; i < n; ++i) p[i] = i + 1;
  p.back() = 0;
  return p;
}

perm rnd(size_t n) {
  perm p(std::from_range, std::views::iota(0UZ, n));
  for (size_t i = 0; i < n; ++i) std::swap(p[i], p[i + rand() % (n - i)]);
  return p;
}

perm inv(const perm& p) {
  perm q(p.size());
  for (size_t i = 0; i < p.size(); ++i) q[p[i]] = i;
  return q;
}

void compose_into(perm& p, const perm& q) {
  for (size_t i = 0; i < p.size(); ++i) p[i] = q[p[i]];
}

perm compose(perm p, const perm& q) {
  compose_into(p, q);
  return p;
}

perm compose(perm p, const auto&... qs) {
  (compose_into(p, qs), ...);
  return p;
}

perm power(perm p, size_t k) {
  perm ret = id(p.size());
  while (k) {
    if (k & 1) compose_into(ret, p);
    p = compose(p, p);
    k >>= 1;
  }
  return ret;
}

perm cycle(size_t n, size_t a, size_t b) {
  // if (a == b) return id(n);
  contract_assert(a < b);
  perm p = id(n);
  for (size_t i = a; i < b; ++i) p[i] = i + 1;
  p[b - 1] = a;
  return p;
}

using cycles = std::vector<std::vector<size_t>>;
cycles cycle_decomp(const perm& p) {
  cycles ret;
  std::vector<bool> seen(p.size(), false);
  for (size_t i = 0; i < p.size(); ++i) {
    if (seen[i]) continue;
    if (p[i] == i) continue;
    ret.emplace_back();
    auto& curr = ret.back();
    for (size_t j = i; !seen[j]; j = p[j]) {
      curr.push_back(j);
      seen[j] = true;
    }
  }
  return ret;
}

bool rand_test(size_t n, auto&& algo) {
  perm p = rnd(n);
  perm x = id(n);
  perm q = inv(p);

  for (size_t i = 0; i < n; ++i)
    for (auto [a, b] : algo(i, p[i], q[i])) std::swap(x[a], x[b]);

  if (x == p) return true;
  ivl::fmt::println("fail with n={}", n);
  ivl::fmt::println("    p={}", p);
  ivl::fmt::println("    q={}", q);
  ivl::fmt::println("    x={}", x);
  ivl::fmt::println();
  return false;
}

bool unique_test(size_t n, auto&& algo) {
  perm p = rnd(n);
  perm q = inv(p);
  perm x = id(n);
  perm y = id(n);

  for (auto i : rnd(n))
    for (auto [a, b] : algo(i, p[i], q[i])) std::swap(x[a], x[b]);
  for (auto i : rnd(n))
    for (auto [a, b] : algo(i, p[i], q[i])) std::swap(y[a], y[b]);

  if (x == y) return true;
  ivl::fmt::println("fail with n={}", n);
  ivl::fmt::println("    p={}  {}", p, cycle_decomp(p));
  ivl::fmt::println("    q={}  {}", q, cycle_decomp(q));
  ivl::fmt::println("    x={}  {}", x, cycle_decomp(x));
  ivl::fmt::println(" != y={}  {}", y, cycle_decomp(y));
  ivl::fmt::println();
  return false;
}

void cycle_exp(size_t n, auto&& algo) {
  perm p = cycle(n);
  perm q = inv(p);
  std::set<perm> outcomes;

  perm order = id(n);
  do {
    perm x = id(n);
    for (auto i : order)
      for (auto [a, b] : algo(i, p[i], q[i])) std::swap(x[a], x[b]);
    outcomes.insert(x);
  } while (std::ranges::next_permutation(order).found);

  ivl::fmt::println("possible outcomes for n={}", n);
  for (auto&& el : outcomes) ivl::fmt::println("    p={}  {}", el, cycle_decomp(el));
  ivl::fmt::println();
}

using swap_seq = std::vector<std::pair<size_t, size_t>>;

swap_seq algo_ip(size_t i, size_t p, size_t q) { return {{i, p}}; }

swap_seq algo_iLp(size_t i, size_t p, size_t q) {
  if (i < p) return {{i, p}};
  else return {};
}

swap_seq algo_weird(size_t i, size_t p, size_t q) {
  if (p == i) return {};
  if (p == q) {
    if (i < p) return {{i, p}};
    return {};
  }
  return {{i, p}, {q, p}};
}

swap_seq algo_pq(size_t i, size_t p, size_t q) { return {{p, q}}; }
swap_seq algo_pLq(size_t i, size_t p, size_t q) {
  if (p < q) return {{p, q}};
  else return {};
}

swap_seq algo_minmax(size_t i, size_t p, size_t q) { return {{std::min({i, p, q}), std::max({i, p, q})}}; }

auto chosen_algo() -> swap_seq (*)(size_t, size_t, size_t) {
  auto name = implicit_value<std::string>("algo", "algo_ip");
  template for (constexpr auto candidate : define_static_array(
                  members_of(^^::, std::meta::access_context::unchecked()) |
                  std::views::filter(std::meta::has_identifier) | std::views::filter(std::meta::is_function)
                )) {
    if constexpr (
      source_location_of(candidate).file_name() == source_location_of(^^chosen_algo).file_name() &&
      is_same_type(type_of(candidate), ^^swap_seq(size_t, size_t, size_t))
    ) {
      if (identifier_of(candidate) == name) return extract<swap_seq (*)(size_t, size_t, size_t)>(candidate);
    }
  }
  contract_assert(false);
  throw;
}

// (0 p0) (1 p1) (2 p2) ...

// algo with only i,p
// prod f(i,pi) == p
// f(0,p0) f(1,p1) ... == p
// f(0,p1) f(1,p0) ... == (0 1) p
// f(0,p1) f(1,p0) ... == p (p0 p1)
// (0 1) f(0,p1) f(1,p0) ... == p
// (0 1) f(0,p1) f(1,p0) == f(0,p0) f(1,p1)
// f(0,p1) f(1,p0) (p0 p1) == f(0,p0) f(1,p1)

// f(a,b) f(c,d) == (a c) f(a,d) f(c,b)
// f(a,b) f(c,d) == f(a,d) f(c,b) (b d)
// A = f(a,b) f(c,d)
// B = f(a,d) f(c,b)
// A == (a c) B == B (b d)
// B^-1 (a c) B == (b d)
// (B(a) B(c)) == (b d)
// (a c) A == A (b d)
// A^-1 (a c) A == (b d)
// (A(a) A(c)) == (b d)

// f(0,0) f(1,1) f(2,2) ... = id

// parity?
// g(i,p) = parity of f(i,p)
// g(0,1) g(1,0) g(2,3) g(3,2) = even
// g(0,1) g(1,2) g(2,3) g(3,0) = odd
// :: g(1,0) != g(1,2) xor g(3,0) != g(3,2)
// g(0,1) g(1,2) g(2,0) g(3,3) = even

// [ (0 1) (2 3) ] [ (0 2) (1 3) ] [ (0 3) (1 2) ] == even
// [ (0 1   2 3) ] [ (0 2   3 1) ] [ (0 3   1 2) ] == odd
// g(0,1) g(1,0) g(2,3) g(3,2) g(0,2) g(2,0) g(1,3) g(3,1) g(0,3) g(3,0) g(2,1) g(1,2) == even
// g(0,1) g(1,2) g(2,3) g(3,0) g(0,2) g(2,3) g(3,1) g(1,0) g(0,3) g(3,1) g(1,2) g(2,0) == odd
// g(0,1) g(1,0)               g(0,2) g(2,0)               g(0,3) g(3,0)               == odd
// g(0,1) g(1,0) g(2,3) g(3,2) g(0,2) g(2,0) g(1,3) g(3,1) g(0,3) g(3,0) g(2,1) g(1,2) == even

// (0 1 2 3 4 5)
// (0 2 . . . .)
// (0 3 . . . .)
// (0 4 . . . .)
// (0 5 . . . .)

// parity explosion:
// cycle=[0, 7, 3, 2, 5, 1, 6, 4]
// cycle=[0, 6, 5, 2, 4, 3, 7, 1]
// cycle=[0, 5, 3, 6, 1, 7, 4, 2]
// cycle=[0, 4, 1, 5, 7, 2, 6, 3]
// cycle=[0, 3, 1, 4, 6, 2, 7, 5]
// cycle=[0, 2, 1, 3, 5, 4, 7, 6]
// cycle=[0, 1, 2, 3, 4, 5, 6, 7]
// therefore f(i,p) does not exist

std::vector<std::vector<char>> search_taken;
std::vector<std::vector<char>> search_taken2;
bool search_for_tilings_rec1(size_t pc);
bool search_for_tilings_rec2(size_t pc, std::vector<size_t>& cycle);

bool search_for_tilings_rec2(size_t pc, std::vector<size_t>& cycle) {
  size_t n = search_taken.size();
  size_t last = cycle.back();
  if (cycle.size() == n) {
    if (search_taken[last][0]) return false;
    search_taken[last][0] = 1;
    if (search_for_tilings_rec1(pc + 1)) return true;
    search_taken[last][0] = 0;
    return false;
  }
  for (size_t i = 1; i < n; ++i)
    if (i != last && !search_taken[last][i] && !search_taken2[pc][i]) {
      search_taken[last][i] = 1;
      search_taken2[pc][i] = 1;
      cycle.push_back(i);
      if (search_for_tilings_rec2(pc, cycle)) return true;
      cycle.pop_back();
      search_taken[last][i] = 0;
      search_taken2[pc][i] = 0;
    }
  return false;
}

bool search_for_tilings_rec1(size_t pc) {
  size_t n = search_taken.size();
  if (pc == n - 1) return true;
  std::vector<size_t> cycle{0, pc + 1};
  search_taken[0][pc + 1] = 1;
  search_taken2[pc][0] = 1;
  search_taken2[pc][pc + 1] = 1;
  if (search_for_tilings_rec2(pc, cycle)) {
    ivl::fmt::println("  cycle={}", cycle);
    return true;
  }
  search_taken[0][pc + 1] = 0;
  search_taken2[pc][0] = 0;
  search_taken2[pc][pc + 1] = 0;
  return false;
}

perm sliding_cycle_comp1(size_t n, size_t m, size_t k) {
  perm p = id(n);
  for (size_t i = 0; i < k; ++i) compose_into(p, cycle(n, i, i + m));
  return p;
}

perm sliding_cycle_comp2(size_t n, size_t m, size_t k) { return power(cycle(n, 0, m + k - 1), k); }

void search_for_tilings(size_t n) {
  search_taken = std::vector(n, std::vector(n, '\0'));
  search_taken2 = std::vector(n, std::vector(n, '\0'));
  // wlog (0 1 2 ... n-1)
  for (size_t i = 0; i + 1 < n; ++i) search_taken[i][i + 1] = 1;
  search_taken[n - 1][0] = 1;
  if (search_for_tilings_rec1(1)) ivl::fmt::println("  cycle={}\n", cycle_decomp(cycle(n))[0]);
  else ivl::fmt::println("no tilings for n={}\n", n);
}

int ivl_main(ivl::cmdline_parsing::implicit) {
  srand(1337);
  if (implicit_flag("rand_test")) {
    size_t maxn = implicit_value("n", 100UZ);
    for (size_t i = 0; i < 1'000'000; ++i) {
      auto n = rand() % maxn + 1;
      if (rand_test(n, chosen_algo())) continue;
      if (n == 1) break;
      maxn = n - 1;
    }
  }
  if (implicit_flag("cycle_exp")) {
    cycle_exp(implicit_value("n", 3UZ), chosen_algo());
  }
  if (implicit_flag("unique_test")) {
    size_t maxn = implicit_value("n", 100UZ);
    for (size_t i = 0; i < 1'000'000; ++i) {
      auto n = rand() % maxn + 1;
      if (unique_test(n, chosen_algo())) continue;
      if (n == 1) break;
      maxn = n - 1;
    }
  }
  if (implicit_flag("search_tilings")) {
    search_for_tilings(implicit_value("n", 4UZ));
  }
  if (implicit_flag("sliding_cycles")) {
    auto n = implicit_value("n", 20UZ);
    auto m = implicit_value("m", 5UZ);
    auto p = id(n);
    auto c = cycle(m);
    auto full = cycle(n);
    auto fulli = inv(full);
    while (c.size() != n) c.push_back(c.size());
    for (size_t i = m; i <= n; ++i) {
      compose_into(p, c);
      // ivl::fmt::println("cycle={} ... p={} ... p={}", cycle_decomp(c)[0], p, cycle_decomp(p));
      ivl::fmt::println("{}", cycle_decomp(p));
      ivl::fmt::println("{}", cycle_decomp(sliding_cycle_comp1(n, m, i - m + 1)));
      ivl::fmt::println("{}", cycle_decomp(sliding_cycle_comp2(n, m, i - m + 1)));
      c = compose(fulli, c, full);
    }
  }
  if (implicit_flag("cycle_power_delta")) {
    auto n = implicit_value("n", 20UZ);
    auto m = implicit_value("m", 5UZ);
    auto delta = implicit_value("delta", 1UZ);
    for (size_t i = 1; i + delta <= n; ++i)
      ivl::fmt::println(
        "i={}\tvs j={}\tdelta={}", i, i + delta,
        cycle_decomp(compose(power(cycle(n, 0, i + delta), m), inv(power(cycle(n, 0, i), m))))
      );
  }
  if (implicit_flag("sliding_delta")) {
    auto n = implicit_value("n", 20UZ);
    auto m = implicit_value("m", 5UZ);
    auto delta = implicit_value("delta", 1UZ);
    for (size_t i = 1; i + delta <= n; ++i)
      ivl::fmt::println(
        "i={}\tvs j={}\tdelta={}", i, i + delta,
        cycle_decomp(compose(power(cycle(n, 0, i + delta), m), inv(power(cycle(n, 0, i), m))))
      );
  }
  return 0;
}
