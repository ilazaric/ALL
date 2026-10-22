#include <ivl/command_line_argument_parsing/implicit_exposed>
#include <ivl/format>
#include <ivl/metaheuristics/pseudo_boolean>
#include <ivl/stl/string>
#include <ivl/utility/tsc>
#include <ivl/utility/tuple_broadcast>
#include "v6"
#include <algorithm>
#include <cassert>
#include <random>
#include <ranges>

struct input_t {
  std::vector<size_t> data;
  size_t m;
};

input_t parse_input(std::string_view sv) {
  std::vector<size_t> a, b;
  size_t cnt = 0;
  for (auto&& piece : ivl::split_py(sv)) {
    contract_assert(!piece.empty());
    contract_assert(piece.back() == 'A' || piece.back() == 'B');
    auto& ref = piece.back() == 'A' ? a : b;
    piece.pop_back();
    size_t p = 0;
    auto res = std::from_chars(piece.data(), piece.data() + piece.size(), p);
    contract_assert(res);
    contract_assert(res.ptr == piece.data() + piece.size());
    for (size_t i = 0; i < p; ++i) ref.push_back(cnt + i);
    cnt += p;
  }
  input_t i;
  i.m = a.size();
  i.data = std::move(a);
  i.data.insert_range(i.data.end(), b);
  return i;
}

struct output_t {
  std::vector<size_t> data;
  ivl::tsc_duration timing;
};

[[gnu::noinline]]
void raw_run(std::vector<size_t>& data, size_t m) {
  ivl::algorithm::monotonic_join_v6(data.begin(), data.begin() + m, data.end());
  asm volatile("" ::"m"(data));
}

double run(const input_t& i) {
  output_t o;
  o.data = i.data;
  auto start = ivl::tsc_now();
  raw_run(o.data, i.m);
  auto end = ivl::tsc_now();
  o.timing = end - start;
  contract_assert(o.data.size() == i.data.size());
  contract_assert(std::ranges::is_sorted(o.data));
  auto c = i.data;
  std::ranges::sort(c);
  contract_assert(c == o.data);
  return (double)o.timing.value;
}

std::vector<double> sample(const input_t& i, size_t rep) {
  std::vector<double> ret(rep);
  for (auto& el : ret) el = run(i);
  return ret;
}

void dump_statistics(const std::vector<double>& samples) {
  auto cnt = samples.size();
  double sum = 0;
  for (auto el : samples) sum += el;
  auto avg = sum / cnt;
  ivl::fmt::println("avg: {}", avg);
  double var = 0;
  for (auto el : samples) var += (el - avg) * (el - avg);
  var /= cnt;
  auto stddev = std::sqrt(var);
  ivl::fmt::println("stddev: {}", stddev);
  auto varcoef = stddev / avg;
  ivl::fmt::println("varcoef: {:.2f}%", varcoef * 100);
}

/*
  perf runs are rather volatile
  avg: 22302.98
  stddev: 4534.639368196769
  varcoef: 20.33%

  TODO
*/

int ivl_main(ivl::cmdline_parsing::implicit) {
  auto input = parse_input(R"input(
3B 1A 6B 1A 2B 3A 1B 1A 2B 1A 1B 5A 2B 1A 3B 2A 1B 1A 3B 1A 2B 1A 2B 2A 1B 1A 4B 1A 2B 5A 2B 1A 1B 2A 2B 1A 4B 2A 1B 1A 1B 2A 2B 2A 1B 1A 1B 1A 1B 3A 2B 2A 1B 1A 3B 1A 2B 5A 3B 1A 1B 1A 2B 2A 2B 2A 1B 2A 3B 3A 2B 1A 1B 1A 1B 2A 1B 1A 1B 1A 2B 2A 1B 2A 1B 3A 3B 1A 1B 4A 2B 1A 2B 2A 4B 1A 5B 1A 1B 1A 1B 3A 1B 1A 2B 2A 2B 1A 1B 2A 4B 3A 2B 1A 3B 1A 8B 3A 1B 1A 4B 1A 2B 1A 1B 3A 4B 2A 3B 4A 1B 2A 1B 1A 3B 3A 3B 1A 2B 2A 1B 1A 2B 3A 2B 1A 1B 1A 1B 3A 1B 4A 1B 6A 6B 4A 1B 4A 2B 1A 3B 1A 2B 2A 1B 1A 2B 1A 3B 1A 1B 6A 1B 2A 1B 1A 1B 2A 2B 1A 1B 2A 2B 3A 1B 2A 4B 1A 4B 1A 1B 3A 6B 1A 1B 2A 7B 1A 2B 1A 4B 1A 1B 1A 4B 3A 2B 3A 7B 1A 1B 1A 1B 2A 3B 1A 2B 2A 1B 1A 1B 2A 4B 1A 2B 2A 1B 1A 1B 2A 1B 2A 2B 1A 1B 2A 1B 1A 1B 1A 1B 1A 1B 5A 3B 2A 1B 5A 1B 1A 2B 4A 1B 1A 1B 1A 3B 1A 2B 2A 1B 1A 2B 2A 1B 6A 3B 1A 1B 1A 2B 1A 1B 4A 2B 2A 1B 1A 5B 1A 2B 2A 2B 1A 1B 1A 3B 2A 2B 1A 2B 1A 1B 1A 2B 1A 1B 2A 2B 2A 2B 1A 1B 4A 1B 1A 1B 1A 4B 3A 1B 2A 1B 1A 1B 3A 2B 1A 2B 5A 3B 5A 1B 4A 1B 3A 1B 6A 6B 1A 2B 1A 2B 1A 1B 1A 5B 1A 1B 1A 1B 1A 1B 1A 2B 1A 3B 2A 1B 3A 2B 2A 1B 1A 1B 1A 1B 2A 1B 1A 1B 1A 2B 1A 1B 3A 2B 2A 1B 1A 1B 1A 2B 4A 1B 2A 1B 1A 1B 2A 1B 2A 4B 2A 1B 2A 2B 1A 2B 4A 3B 1A 1B 1A 1B 1A 4B 1A 1B 1A 1B 1A 1B 1A 1B 2A 2B 1A 4B 4A 1B 1A 1B 2A 1B 1A 2B 2A 2B 1A 1B 1A 4B 3A 1B 1A 3B 4A 2B 3A 1B 1A 1B 2A 1B 1A 2B 1A 1B 5A 1B 3A 2B 1A 1B 2A 1B 2A 2B 1A 4B 5A 1B 3A 1B 3A 2B 7A 4B 2A 2B 4A 5B 2A 1B 2A 1B 1A 1B 2A 1B 3A 3B 2A 2B 4A 2B 1A 1B 1A 2B 1A 2B 2A 4B 2A 3B 1A 3B 1A 1B 5A 3B 2A 1B 2A 1B 1A 3B 2A 1B 1A 1B 1A 2B 1A 1B 3A 3B 2A 4B 1A 1B 3A 1B 1A 3B 1A 4B
)input");
  contract_assert(input.data.size() == 1000);
  auto s = sample(input, 100);
  dump_statistics(s);
  return 0;
}
