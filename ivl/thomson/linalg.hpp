#pragma once

#include <span>
#include <vector>

// template<typename T>
// struct vector;

// template<typename>
// concept vector_space = true;

// template<typename... Ts>
// struct cartesian_product_t {
//   Ts... ts;
// };

// // static extent should not increase size
// // dynamic extent has to increase size
// // complex shape could be all-static or some-dynamic
// // all-static does not exist in class
// // some-dynamic exists in class
// // should be min size that way

// template<auto... extents>
// struct sum_extent {

// };

// template<auto... shapes>
// struct product_extent {

// };

// struct

// // shape | data

// // A -> B
// // A -> B -> 1
// // M(a).b
// // V ~? V* should be

// auto cartesian_product(vector_space auto, vector_space auto) {

// }

// auto tensor_product(vector_space auto, vector_space auto);

// auto cartesian_product(auto... as) { return std::tuple{std::move(as)...}; }

// auto tensor_product(auto a, auto b) {

// }

// template<typename MD>
// class owned_mdspan {
//   MD md;
// };

// invariant: data.size() == (shape[i] * ...)
struct mdarray_ref {
  std::span<std::size_t> shape;
  std::span<double> data;
};

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

  double extract_number() const pre(rank() == 0) {
    contract_assert(data.size() == 1);
    return data[0];
  }

  std::vector<double> extract_vector() const pre(rank() == 1) { return data; }

  void match_size() {
    std::size_t sz = 1;
    for (std::size_t i = 0; i < rank(); ++i) {
      bool overflow = __builtin_mul_overflow(sz, extent(i), &sz);
      contract_assert(!overflow);
    }
    data.resize(sz, 0.0);
    contract_assert(check_size_invariant());
  }

  // e_i --> data[Li...L(i+1)] (L == size / extent(0))
  mdarray operator()(std::span<double> vec) const pre(rank() > 0 && extent(0) == vec.size()) {
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
};

// A -> B -> F
// A -> B

// the more natural direction, compose(a,b) = b o a
// a : X -> Y -> Z (-> F)
// b : Z -> W -> T (-> F)
// compose(a,b) : X -> Y -> W -> T (-> F)
auto compose(const mdarray& a, const mdarray& b) {
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
