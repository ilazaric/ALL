#include <algorithm>
#include <exception>
#include <print>
#include <vector>

#include <cxxabi.h>
#include <dlfcn.h>

size_t gcd(size_t a, size_t b) {
  while (a) b %= a, std::swap(a, b);
  return b;
}

size_t lcm(size_t a, size_t b) {
  a /= gcd(a, b);
  size_t res = a * b;
  if (res / a != b) throw std::runtime_error{"overflow"};
  return res;
}

struct type_spec {
  size_t size;
  size_t alignment;
  auto operator<=>(const type_spec&) const = default;
};

void align(size_t& value, size_t alignment) {
  size_t m = value % alignment;
  if (m == 0) return;
  value += alignment - m;
  if (value < alignment) throw std::runtime_error{"overflow"};
}

size_t struct_size(const std::vector<type_spec>& vec) {
  size_t res = 0;
  for (auto [size, alignment] : vec) {
    align(res, alignment);
    res += size;
    // res.alignment = lcm(res.alignment, alignment);
  }
  // align(res.size, res.alignment);
  return res;
}

std::pair<size_t, std::vector<type_spec>> min_size(std::vector<type_spec> vec) {
  size_t res = -1;
  std::vector<type_spec> layout;
  std::ranges::sort(vec);
  do {
    auto curr = struct_size(vec);
    if (curr < res) res = curr, layout = vec;
  } while (std::ranges::next_permutation(vec).found);
  return {res, layout};
}

std::pair<size_t, std::vector<type_spec>> greedy_size(std::vector<type_spec> vec) {
  std::ranges::sort(vec, std::greater<>{}, &type_spec::alignment);
  return {struct_size(vec), vec};
}

std::vector<type_spec> create_random_spec(int n, int size_limit, int alignment_limit) {
  n = rand() % n + 1;
  std::vector<type_spec> res;
  std::vector<int> powers{1};
  while (powers.back() * 2 <= alignment_limit) powers.push_back(powers.back() * 2);
  for (int i = 0; i < n; ++i) {
    res.emplace_back(rand() % size_limit + 1, powers[rand() % powers.size()]);
    // res.emplace_back(rand() % size_limit + 1, rand() % alignment_limit + 1);
    // align(res.back().size, res.back().alignment);
  }
  return res;
}

// IVL add_compiler_flags("-rdynamic")
std::string function_name(auto ptr) {
  Dl_info info;
  dladdr(reinterpret_cast<void*>(ptr), &info);
  return abi::__cxa_demangle(info.dli_sname, NULL, NULL, NULL);
}

void find_mismatch(auto algo1, auto algo2) {
  std::println("comparing:");
  std::println(" - {}", function_name(algo1));
  std::println(" - {}", function_name(algo2));
  int n = 8;
  int size_limit = 100;
  int alignment_limit = 20;

  std::vector<type_spec> res;

  auto attempt = [&] {
    auto spec = create_random_spec(n, size_limit, alignment_limit);
    auto s1 = algo1(spec);
    auto s2 = algo2(spec);
    if (s1.first == s2.first) {
      std::print(".");
      return true;
    }
    std::println();
    res = spec;
    return false;
  };

  auto shrink_to_fit = [&] {
    bool changed = false;
    auto modify = [&](auto& x, auto y) {
      if (x == y) return;
      changed = true;
      x = y;
    };
    modify(n, res.size());
    modify(size_limit, std::ranges::max(res, {}, &type_spec::size).size);
    modify(alignment_limit, std::ranges::max(res, {}, &type_spec::alignment).alignment);
    return changed;
  };

  while (attempt());
  std::println("found mismatch, minimizing ...");

  for (int i = 0; i < 10000; ++i)
    if (attempt() && shrink_to_fit()) i = 0;

  std::println();
  auto s1 = algo1(res);
  auto s2 = algo2(res);
  auto dump_sol = [](const auto& x) {
    std::println(" - size: {}", x.first);
    std::println(" - layout: {{");
    for (auto [size, alignment] : x.second) std::println("  {{{}, {}}},", size, alignment);
    std::println("}}");
  };
  std::println("algo1 ({}):", function_name(algo1));
  dump_sol(s1);
  std::println("algo2 ({}):", function_name(algo2));
  dump_sol(s2);
}

std::pair<size_t, std::vector<type_spec>> theorical_min_size(const std::vector<type_spec>& vec) {
  size_t sum = 0;
  size_t alignment = 1;
  for (auto el : vec) sum += el.size, alignment = lcm(alignment, el.alignment);
  align(sum, alignment);
  return {sum, {}};
}

std::pair<size_t, std::vector<type_spec>> optimizing_swap_size(std::vector<type_spec> vec) {
  auto curr = struct_size(vec);
retry:
  for (int i = 0; i + 1 < vec.size(); ++i) {
    std::swap(vec[i], vec[i + 1]);
    auto nxt = struct_size(vec);
    if (nxt < curr) {
      curr = nxt;
      goto retry;
    }
    std::swap(vec[i], vec[i + 1]);
  }
  return {curr, vec};
}

std::pair<size_t, std::vector<type_spec>> optimizing_reverse_size(std::vector<type_spec> vec) {
  auto curr = struct_size(vec);
retry:
  for (int i = 0; i <= vec.size(); ++i)
    for (int j = 0; j < i; ++j) {
      std::reverse(vec.begin() + j, vec.begin() + i);
      auto nxt = struct_size(vec);
      if (nxt < curr) {
        curr = nxt;
        goto retry;
      }
      std::reverse(vec.begin() + j, vec.begin() + i);
    }
  return {curr, vec};
}

std::pair<size_t, std::vector<type_spec>> optimizing_swap2_size(std::vector<type_spec> vec) {
  auto curr = struct_size(vec);
retry:
  for (int i = 0; i < vec.size(); ++i)
    for (int j = 0; j < i; ++j) {
      std::swap(vec[i], vec[j]);
      auto nxt = struct_size(vec);
      if (nxt < curr) {
        curr = nxt;
        goto retry;
      }
      std::swap(vec[i], vec[j]);
    }
  return {curr, vec};
}

/*
  assume alignment is power-of-2 for now
  dont assume size is divisible by alignment
  size is interpreted as sizeof(T)-tail_padding(T)

  align sum <=> sum += -sum % alignment
  sum += size
  repeat

  the size penalty is easy to think about i guess
  the alignment penalty is stranger though
  [0, alignment-1]

  max alignment & size divisible by alignment -> i dont care about it
  more generally, size divisible by max alignment -> i dont care about it
  in fact, every size can be reduced by max alignment

  alignment = 1 --> simple (but not ignorable)


 */

int main() {
  srand(42);
  // find_mismatch(min_size, greedy_size);
  // find_mismatch(min_size, theorical_min_size);
  // find_mismatch(min_size, optimizing_swap_size);
  find_mismatch(min_size, optimizing_reverse_size);
}
