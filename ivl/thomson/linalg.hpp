#pragma once

#include <ivl/meta>
#include <memory>
#include <span>
#include <variant>
#include <vector>

// using extent_t = std::size_t;
// using shape_t = std::vector<extent_t>;

// f  : R -> R
// Df : R -> L(R,R)

// R != R^1

// {}     --- R
// {N}    --- R^N (-> R)
// {A, B} --- R^A -> R^B (-> R)
// {????} --- (R^A -> R^B) -> R^C (-> R)
// struct shape_type {};

// i must differentiate between A and A -> R i think

// shape:
// * field
// * N-dimensional vector space
// * cartesian product
// * linear operator

struct shape_t {
  struct field_t {
    auto operator<=>(const field_t&) const = default;
  };
  struct vector_t {
    std::size_t n;
    auto operator<=>(const vector_t&) const = default;
  };
  struct cartesian_t {
    std::vector<shape_t> shapes;
    auto operator<=>(const cartesian_t&) const = default;
  };
  struct tensor_t {
    std::vector<shape_t> shapes;
    auto operator<=>(const tensor_t& o) const = default;
  };

  std::size_t total_size;
  std::variant<field_t, vector_t, cartesian_t, tensor_t> data;

  auto operator<=>(const shape_t&) const = default;

  bool is_field() const { return std::holds_alternative<field_t>(data); }
  bool is_vector() const { return std::holds_alternative<vector_t>(data); }
  bool is_cartesian_product() const { return std::holds_alternative<cartesian_t>(data); }
  bool is_linear_operator() const { return std::holds_alternative<tensor_t>(data); }

  std::size_t size() const { return total_size; }

  decltype(auto) cartesian_product_elements(this auto&& self) {
    // not pre bc of gcc bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=125733
    contract_assert(self.is_cartesian_product());
    return std::forward_like<decltype(self)>(std::get<cartesian_t>(self.data).shapes);
  }
  decltype(auto) linear_operator_from(this auto&& self) {
    // not pre bc of gcc bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=125733
    contract_assert(self.is_linear_operator());
    return std::forward_like<decltype(self)>(std::get<tensor_t>(self.data).shapes[0]);
  }
  decltype(auto) linear_operator_to(this auto&& self) {
    // not pre bc of gcc bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=125733
    contract_assert(self.is_linear_operator());
    return std::forward_like<decltype(self)>(std::get<tensor_t>(self.data).shapes[1]);
  }
  decltype(auto) vector_size(this auto&& self) {
    // not pre bc of gcc bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=125733
    contract_assert(self.is_vector());
    return std::forward_like<decltype(self)>(std::get<vector_t>(self.data).n);
  }
  decltype(auto) final_to(this auto&& self) {
    auto* curr = &self;
    while (curr->is_linear_operator()) curr = &curr->linear_operator_to();
    return std::forward_like<decltype(self)>(*curr);
  }
  std::size_t linear_operator_rank() const {
    std::size_t ret = 0;
    for (auto* curr = this; curr->is_linear_operator(); ++ret, curr = &curr->linear_operator_to());
    return ret;
  }

  template<auto Member>
  std::size_t fixup_size_generic() {
    if (is_field()) return total_size = 1;
    if (is_vector()) return total_size = std::get<vector_t>(data).n;
    if (is_cartesian_product()) {
      total_size = 0;
      for (auto&& el : cartesian_product_elements()) total_size += (el.*Member)();
      return total_size;
    }
    if (is_linear_operator()) return total_size = (linear_operator_from().*Member)() * (linear_operator_to().*Member)();
    contract_assert(false);
  }

  std::size_t fixup_size_deep() { return fixup_size_generic<&shape_t::fixup_size_deep>(); }
  std::size_t fixup_size_shallow() { return fixup_size_generic<&shape_t::size>(); }

  static shape_t field() { return shape_t{.total_size = 1, .data{field_t{}}}; }
  static shape_t vector(std::size_t n) { return shape_t{.total_size = n, .data{vector_t{n}}}; }

  template<ivl::meta::is_unqualified<shape_t>... Ts>
  static shape_t cartesian_product(Ts&&... args) {
    return shape_t{
      .total_size = (args.size() + ...),
      .data{cartesian_t{std::forward<decltype(args)>(args)...}},
    };
  }

  template<ivl::meta::is_unqualified<shape_t> T>
  static shape_t linear_operator(T&& from) {
    return std::forward<T>(from);
  }

  template<ivl::meta::is_unqualified<shape_t> T, ivl::meta::is_unqualified<shape_t> U>
  static shape_t linear_operator(T&& from, U&& to) {
    return shape_t{
      .total_size = from.size() * to.size(),
      .data{tensor_t{.shapes{
        std::forward<T>(from),
        std::forward<T>(to),
      }}},
    };
  }

  template<ivl::meta::is_unqualified<shape_t> T, ivl::meta::is_unqualified<shape_t>... Us>
    requires(sizeof...(Us) >= 2)
  static shape_t linear_operator(T&& from, Us&&... to) {
    return linear_operator(std::forward<T>(from), linear_operator(std::forward<Us>(to)...));
  }
};

// std::strong_ordering shape_t::tensor_t::operator<=>(const shape_t::tensor_t& o) const {
//   return std::tie(*from, *to) <=> std::tie(*o.from, *o.to);
// }
// bool shape_t::tensor_t::operator==(const shape_t::tensor_t& o) const {
//   return std::tie(*from, *to) == std::tie(*o.from, *o.to);
// }

// struct ranked {
//   std::size_t index;
//   std::size_t rank;
// };

// TODO: different shape might make more sense
// ....: instead of {extent, extent, ...}
// ....: it could be {product[0..], product[1..], ...}
// ....: that way everything is cheap
// ....: size() == shape[0]
// ....: extent(i) == shape[i] / shape[i+1]
// ....: size of suffix: shape[i]

// invariant: data.size() == (shape[i] * ...)
struct mdarray_cref {
  // std::span<const std::size_t> shape;
  const shape_t* shape;
  std::span<const double> data;

  // std::size_t rank() const { return shape.size(); }
  std::size_t size() const { return data.size(); }
  // std::size_t extent(std::size_t i) const pre(i < rank()) { return shape[i]; }

  double extract_number() const pre(shape->is_field()) {
    contract_assert(size() == 1);
    return data[0];
  }

  mdarray_cref operator[](std::size_t i) const pre(
    shape->is_linear_operator() && i < shape->linear_operator_from().size()
    // rank() > 0 && i < extent(0)
  ) {
    std::size_t foo = shape->linear_operator_to().size();
    return mdarray_cref{&shape->linear_operator_to(), data.subspan(i * foo, foo)};
  }

  // mdarray_cref operator[](ranked r) const pre(rank() >= r.rank) {
  //   std::size_t prefix = 1;
  //   for (std::size_t i = 0; i < r.rank; ++i) prefix *= shape[i];
  //   contract_assert(r.index < prefix);
  //   std::size_t foo = data.size() / prefix;
  //   return mdarray_cref{shape.subspan(r.rank), data.subspan(r.index * foo, foo)};
  // }
};

// invariant: data.size() == (shape[i] * ...)
struct mdarray_ref {
  // std::span<const std::size_t> shape;
  const shape_t* shape;
  std::span<double> data;

  // std::size_t rank() const { return shape.size(); }
  std::size_t size() const { return data.size(); }
  // std::size_t extent(std::size_t i) const pre(i < rank()) { return shape[i]; }

  double extract_number() const pre(shape->is_field()) {
    contract_assert(size() == 1);
    return data[0];
  }

  mdarray_ref operator[](std::size_t i) const pre(
    shape->is_linear_operator() && i < shape->linear_operator_from().size()
    // rank() > 0 && i < extent(0)
  ) {
    std::size_t foo = shape->linear_operator_to().size();
    return mdarray_ref{&shape->linear_operator_to(), data.subspan(i * foo, foo)};
  }

  // mdarray_ref operator[](ranked r) const pre(rank() >= r.rank) {
  //   std::size_t prefix = 1;
  //   for (std::size_t i = 0; i < r.rank; ++i) prefix *= shape[i];
  //   contract_assert(r.index < prefix);
  //   std::size_t foo = data.size() / prefix;
  //   return mdarray_ref{shape.subspan(r.rank), data.subspan(r.index * foo, foo)};
  // }

  double& operator=(double x) pre(shape->is_field()) {
    contract_assert(data.size() == 1);
    return data[0] = x;
  }
};

// invariant: data.size() == (shape[i] * ...)
struct mdarray {
  shape_t shape;
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

  // std::size_t rank() const { return shape.size(); }
  std::size_t size() const { return data.size(); }
  // std::size_t extent(std::size_t i) const pre(i < rank()) { return shape[i]; }

  bool check_size_invariant() const {
    // std::size_t shape_size = 1;
    // for (auto extent : shape) {
    //   bool overflow = __builtin_mul_overflow(shape_size, extent, &shape_size);
    //   if (overflow) return false;
    // }
    // return shape_size == data.size();
    return shape.size() == data.size();
  }

  void match_size() {
    // std::size_t sz = 1;
    // for (std::size_t i = 0; i < rank(); ++i) {
    //   bool overflow = __builtin_mul_overflow(sz, extent(i), &sz);
    //   contract_assert(!overflow);
    // }
    data.resize(shape.size(), 0.0);
    contract_assert(check_size_invariant());
  }

  explicit mdarray(shape_t shape) : shape(std::move(shape)) {
    // TODO: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=125645
    // contract_assert(((extents > 0) && ...));
    match_size();
  }

  double extract_number() const pre(shape.is_field()) {
    contract_assert(data.size() == 1);
    return data[0];
  }

  std::vector<double> extract_vector() const pre(shape.is_vector()) { return data; }

  // e_i --> data[Li...L(i+1)] (L == size / extent(0))
  mdarray
  operator()(const mdarray& arg) const pre(shape.is_linear_operator() && shape.linear_operator_from() == arg.shape) {
    std::size_t n = arg.size();
    mdarray ret(shape.linear_operator_to());
    // for (std::size_t i = 1; i < rank(); ++i) ret.shape.push_back(extent(i));
    // ret.match_size();
    auto rem = ret.size();
    for (std::size_t jk = 0; jk < data.size(); ++jk) {
      std::size_t j = jk % rem;
      std::size_t k = jk / rem;
      ret.data[j] += data[jk] * arg.data[k];
    }
    return ret;
  }

  mdarray_ref as_ref() { return mdarray_ref{.shape{&shape}, .data{data}}; }
  mdarray_cref as_cref() const { return mdarray_cref{.shape{&shape}, .data{data}}; }

  operator mdarray_ref() { return as_ref(); }
  operator mdarray_cref() const { return as_cref(); }

  mdarray_ref operator[](std::size_t i) { return as_ref()[i]; }
  // mdarray_ref operator[](ranked r) { return as_ref()[r]; }

  mdarray_cref operator[](std::size_t i) const { return as_cref()[i]; }
  // mdarray_cref operator[](ranked r) const { return as_cref()[r]; }

  double& operator=(double x) pre(shape.is_field()) {
    contract_assert(data.size() == 1);
    return data[0] = x;
  }
};

// A -> B -> F
// A -> B

// the more natural direction, compose(a,b) = b o a
// a : X -> Y -> Z (-> F)
// b : Z -> W -> T (-> F)
// compose(a,b) : X -> Y -> W -> T (-> F)
mdarray compose(mdarray_cref a, mdarray_cref b);
// {
//   // LOG(std::format("{}", a.shape));
//   // LOG(std::format("{}", b.shape));
//   // TODO: pre
//   contract_assert(a.rank() > 0);
//   contract_assert(b.rank() > 0);
//   contract_assert(a.extent(a.rank() - 1) == b.extent(0));
//   mdarray ret;
//   for (std::size_t i = 0; i < a.rank() - 1; ++i) ret.shape.push_back(a.extent(i));
//   for (std::size_t i = 1; i < b.rank(); ++i) ret.shape.push_back(b.extent(i));
//   ret.match_size();
//   std::size_t overlap = b.extent(0);
//   std::size_t a_ex = a.size() / overlap;
//   std::size_t b_ex = b.size() / overlap;
//   // TODO: perf
//   for (std::size_t ai = 0; ai < a_ex; ++ai)
//     for (std::size_t ov = 0; ov < overlap; ++ov)
//       for (std::size_t bi = 0; bi < b_ex; ++bi)
//         ret.data[ai * b_ex + bi] += a.data[ai * overlap + ov] * b.data[ov * b_ex + bi];
//   return ret;
// }
