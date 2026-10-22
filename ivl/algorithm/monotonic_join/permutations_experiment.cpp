#include <ivl/command_line_argument_parsing/implicit_exposed>
#include <ivl/format>
#include <ivl/permutation>
#include <algorithm>
#include <ranges>
#include <set>
#include <vector>

using perm = ivl::permutation;

bool rand_test(size_t n, auto&& algo) {
  perm p = perm::rnd(n);
  perm x;
  perm q = p.inverse();

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
  perm p = perm::rnd(n);
  perm q = p.inverse();
  perm x;
  perm y;

  for (auto i : perm::rnd(n))
    for (auto [a, b] : algo(i, p[i], q[i])) std::swap(x[a], x[b]);
  for (auto i : perm::rnd(n))
    for (auto [a, b] : algo(i, p[i], q[i])) std::swap(y[a], y[b]);

  if (x == y) return true;
  ivl::fmt::println("fail with n={}", n);
  ivl::fmt::println("    p={}  {}", p, p.cycle_decomp());
  ivl::fmt::println("    q={}  {}", q, p.cycle_decomp());
  ivl::fmt::println("    x={}  {}", x, p.cycle_decomp());
  ivl::fmt::println(" != y={}  {}", y, p.cycle_decomp());
  ivl::fmt::println();
  return false;
}

void cycle_exp(size_t n, auto&& algo) {
  perm p = perm::cycle(n);
  perm q = p.inverse();
  std::set<perm> outcomes;

  perm order;
  order.extend(n);
  do {
    perm x;
    for (auto i : order)
      for (auto [a, b] : algo(i, p[i], q[i])) std::swap(x[a], x[b]);
    outcomes.insert(x);
  } while (std::ranges::next_permutation(order.v).found);

  ivl::fmt::println("possible outcomes for n={}", n);
  for (auto&& el : outcomes) ivl::fmt::println("    p={}  {}", el, el.cycle_decomp());
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
  perm p;
  for (size_t i = 0; i < k; ++i) p *= perm::cycle(i, i + m);
  return p;
}

perm sliding_cycle_comp2(size_t n, size_t m, size_t k) { return perm::cycle(m + k - 1).power(k); }

void search_for_tilings(size_t n) {
  search_taken = std::vector(n, std::vector(n, '\0'));
  search_taken2 = std::vector(n, std::vector(n, '\0'));
  // wlog (0 1 2 ... n-1)
  for (size_t i = 0; i + 1 < n; ++i) search_taken[i][i + 1] = 1;
  search_taken[n - 1][0] = 1;
  if (search_for_tilings_rec1(1)) ivl::fmt::println("  cycle={}\n", perm::cycle(n).cycle_decomp()[0]);
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
    perm p;
    auto c = perm::cycle(m);
    auto full = perm::cycle(n);
    auto fulli = full.inverse();
    for (size_t i = m; i <= n; ++i) {
      p *= c;
      // ivl::fmt::println("cycle={} ... p={} ... p={}", cycle_decomp(c)[0], p, cycle_decomp(p));
      ivl::fmt::println("{}", p.cycle_decomp());
      ivl::fmt::println("{}", sliding_cycle_comp1(n, m, i - m + 1).cycle_decomp());
      ivl::fmt::println("{}", sliding_cycle_comp2(n, m, i - m + 1).cycle_decomp());
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
        (perm::cycle(i + delta).power(m) * perm::cycle(i).power(m).inverse()).cycle_decomp()
      );
  }
  if (implicit_flag("sliding_delta")) {
    auto n = implicit_value("n", 20UZ);
    auto m = implicit_value("m", 5UZ);
    auto delta = implicit_value("delta", 1UZ);
    for (size_t i = 1; i + delta <= n; ++i)
      ivl::fmt::println(
        "i={}\tvs j={}\tdelta={}", i, i + delta,
        (perm::cycle(i + delta).power(m) * perm::cycle(i).power(m).inverse()).cycle_decomp()
      );
  }
  if (implicit_flag("interleaved")) {
    auto n = implicit_value("n", 20UZ);
    perm p;
    for (size_t i = 0; i < n; ++i) {
      p *= perm::cycle(2 * i + 1, n + i + 1);
      // ivl::fmt::println("{}", p);
      ivl::fmt::println("{}", p.cycle_decomp());
    }
  }
  return 0;
}
