#pragma once

#include <span>
#include <vector>

struct ranked {
  std::size_t index;
  std::size_t rank;
};

// TODO: different shape might make more sense
// ....: instead of {extent, extent, ...}
// ....: it could be {product[0..], product[1..], ...}
// ....: that way everything is cheap
// ....: size() == shape[0]
// ....: extent(i) == shape[i] / shape[i+1]
// ....: size of suffix: shape[i]

// invariant: data.size() == (shape[i] * ...)
struct mdarray_cref {
  std::span<const std::size_t> shape;
  std::span<const double> data;

  std::size_t rank() const { return shape.size(); }
  std::size_t size() const { return data.size(); }
  std::size_t extent(std::size_t i) const pre(i < rank()) { return shape[i]; }

  double extract_number() const pre(rank() == 0) {
    contract_assert(size() == 1);
    return data[0];
  }

  mdarray_cref operator[](std::size_t i) const pre(rank() > 0 && i < extent(0)) {
    std::size_t foo = data.size() / shape[0];
    return mdarray_cref{shape.subspan(1), data.subspan(i * foo, foo)};
  }

  mdarray_cref operator[](ranked r) const pre(rank() >= r.rank) {
    std::size_t prefix = 1;
    for (std::size_t i = 0; i < r.rank; ++i) prefix *= shape[i];
    contract_assert(r.index < prefix);
    std::size_t foo = data.size() / prefix;
    return mdarray_cref{shape.subspan(r.rank), data.subspan(r.index * foo, foo)};
  }
};

// invariant: data.size() == (shape[i] * ...)
struct mdarray_ref {
  std::span<const std::size_t> shape;
  std::span<double> data;

  std::size_t rank() const { return shape.size(); }
  std::size_t size() const { return data.size(); }
  std::size_t extent(std::size_t i) const pre(i < rank()) { return shape[i]; }

  double extract_number() const pre(rank() == 0) {
    contract_assert(size() == 1);
    return data[0];
  }

  mdarray_ref operator[](std::size_t i) const pre(rank() > 0 && i < extent(0)) {
    std::size_t foo = data.size() / shape[0];
    return mdarray_ref{shape.subspan(1), data.subspan(i * foo, foo)};
  }

  mdarray_ref operator[](ranked r) const pre(rank() >= r.rank) {
    std::size_t prefix = 1;
    for (std::size_t i = 0; i < r.rank; ++i) prefix *= shape[i];
    contract_assert(r.index < prefix);
    std::size_t foo = data.size() / prefix;
    return mdarray_ref{shape.subspan(r.rank), data.subspan(r.index * foo, foo)};
  }

  double& operator=(double x) pre(rank() == 0) {
    contract_assert(data.size() == 1);
    return data[0] = x;
  }
};

struct shaped_t {};
inline constexpr shaped_t shaped;

// invariant: data.size() == (shape[i] * ...)
struct mdarray {
  std::vector<std::size_t> shape;
  std::vector<double> data;

  // 0,0,...,0,0 -> 0
  // 0,0,...,0,1 -> 1
  // 0,0,...,0,2 -> 2
  // ...
  // 0,0,...,1,0 -> N

  mdarray() = default;
  mdarray(const mdarray&) = default;
  mdarray(mdarray&&) = default;
  mdarray& operator=(const mdarray&) = default;
  mdarray& operator=(mdarray&&) = default;
  ~mdarray() = default;

  std::size_t rank() const { return shape.size(); }
  std::size_t size() const { return data.size(); }
  std::size_t extent(std::size_t i) const pre(i < rank()) { return shape[i]; }

  bool check_size_invariant() const {
    std::size_t shape_size = 1;
    for (auto extent : shape) {
      bool overflow = __builtin_mul_overflow(shape_size, extent, &shape_size);
      if (overflow) return false;
    }
    return shape_size == data.size();
  }

  void match_size() {
    std::size_t sz = 1;
    for (std::size_t i = 0; i < rank(); ++i) {
      bool overflow = __builtin_mul_overflow(sz, extent(i), &sz);
      contract_assert(!overflow);
    }
    data.resize(sz, 0.0);
    contract_assert(check_size_invariant());
  }

  explicit mdarray(shaped_t, auto... extents) : shape{(std::size_t)extents...} {
    // TODO: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=125645
    contract_assert(((extents > 0) && ...));
    match_size();
  }

  double extract_number() const pre(rank() == 0) {
    contract_assert(data.size() == 1);
    return data[0];
  }

  std::vector<double> extract_vector() const pre(rank() == 1) { return data; }

  // e_i --> data[Li...L(i+1)] (L == size / extent(0))
  mdarray operator()(std::span<const double> vec) const pre(rank() > 0 && extent(0) == vec.size()) {
    std::size_t n = vec.size();
    mdarray ret;
    for (std::size_t i = 1; i < rank(); ++i) ret.shape.push_back(extent(i));
    ret.match_size();
    auto rem = ret.size();
    for (std::size_t jk = 0; jk < data.size(); ++jk) {
      std::size_t j = jk % rem;
      std::size_t k = jk / rem;
      ret.data[j] += data[jk] * vec[k];
    }
    return ret;
  }

  mdarray_ref as_ref() { return mdarray_ref{.shape{shape}, .data{data}}; }
  mdarray_cref as_cref() const { return mdarray_cref{.shape{shape}, .data{data}}; }

  operator mdarray_ref() { return as_ref(); }
  operator mdarray_cref() const { return as_cref(); }

  mdarray_ref operator[](std::size_t i) { return as_ref()[i]; }
  mdarray_ref operator[](ranked r) { return as_ref()[r]; }

  mdarray_cref operator[](std::size_t i) const { return as_cref()[i]; }
  mdarray_cref operator[](ranked r) const { return as_cref()[r]; }
};

// A -> B -> F
// A -> B

// the more natural direction, compose(a,b) = b o a
// a : X -> Y -> Z (-> F)
// b : Z -> W -> T (-> F)
// compose(a,b) : X -> Y -> W -> T (-> F)
auto compose(const mdarray& a, const mdarray& b) {
  // LOG(std::format("{}", a.shape));
  // LOG(std::format("{}", b.shape));
  // TODO: pre
  contract_assert(a.rank() > 0);
  contract_assert(b.rank() > 0);
  contract_assert(a.extent(a.rank() - 1) == b.extent(0));
  mdarray ret;
  for (std::size_t i = 0; i < a.rank() - 1; ++i) ret.shape.push_back(a.extent(i));
  for (std::size_t i = 1; i < b.rank(); ++i) ret.shape.push_back(b.extent(i));
  ret.match_size();
  std::size_t overlap = b.extent(0);
  std::size_t a_ex = a.size() / overlap;
  std::size_t b_ex = b.size() / overlap;
  // TODO: perf
  for (std::size_t ai = 0; ai < a_ex; ++ai)
    for (std::size_t ov = 0; ov < overlap; ++ov)
      for (std::size_t bi = 0; bi < b_ex; ++bi)
        ret.data[ai * b_ex + bi] += a.data[ai * overlap + ov] * b.data[ov * b_ex + bi];
  return ret;
}
