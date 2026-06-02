#pragma once

#include <exception>
#include <ostream>
#include <type_traits>
#include <utility>

// TODO-think: noexcept?
// TODO: analyze how horrible are compilation errors
namespace ivl::pointwise {

// TODO: test this
class PointwiseException : public std::exception {
public:
  // `override` needs to come last
  const char *what() const noexcept override {
    // TODO-think: we could report more details maybe
    // though that would require allocation?
    return "Argument length mismatch -- pointwise operations have to be "
           "performed on same-length/size containers";
  }
};

// `is_container[_v]` is a requirement for our operators
// it just checks if member functions `begin` and `end`
// are implemented
// TODO-think: should this eliminate certain containers like `std::string`?
// what about `std::map`, which just breaks everything here
template <typename T, typename = void> struct is_container : std::false_type {};

template <typename T>
struct is_container<T, std::void_t<decltype(begin(std::declval<T>())),
                                   decltype(end(std::declval<T>()))>>
    : std::true_type {};

template <typename T>
inline constexpr bool is_container_v = is_container<T>::value;

template <typename A, typename B = A, typename C = A> struct noop {
  using type = C;
};

// this generates the assignment operators (+=, -=, ...)
#define IVL_OPS_GENERATOR_ASSIGNMENT(op)                                       \
  template <typename A, typename B>                                            \
  std::enable_if<is_container_v<A> && is_container_v<B>, A &>::type            \
  operator op(A &a, const B &b) {                                              \
    auto it_a = a.begin();                                                     \
    auto it_b = b.begin();                                                     \
    while (it_a != a.end() && it_b != b.end()) {                               \
      *it_a op *it_b;                                                          \
      ++it_a;                                                                  \
      ++it_b;                                                                  \
    }                                                                          \
    if (it_a != a.end() || it_b != b.end())\
      throw PointwiseException{};\
    return a;                                                                  \
  }

// this generates non-assignment operators
// via the assignment versions (+, -, ...)
#define IVL_OPS_GENERATOR_REGULAR(op)                                          \
  template <typename A, typename B>                                            \
  std::enable_if<is_container_v<std::remove_cvref_t<A>> && is_container_v<B>,  \
                 std::remove_cvref_t<A>>::type                                 \
  operator op(A &&a, const B &b) {                                             \
    std::remove_cvref_t<A> c{std::forward<A>(a)};                              \
    return operator op##=(c, b);                                               \
  }

// TODO: doc
#define IVL_OPS_GENERATOR_UNARY_PRE(op)                                        \
  template <typename T>                                                        \
  std::enable_if<is_container_v<T>, T &>::type operator op(T &t) {             \
    for (auto &el : t)                                                         \
      op el;                                                                   \
    return t;                                                                  \
  }

// TODO: doc
// TODO-think: should this do something else for rvalue-refs?
#define IVL_OPS_GENERATOR_UNARY_POST(op)                                       \
  template <typename T>                                                        \
  std::enable_if<is_container_v<std::remove_cvref_t<T>>,                       \
                 std::remove_cvref_t<T>>::type                                 \
  operator op(T &t, int) {                                                     \
    std::remove_cvref_t<T> ret{t};                                             \
    for (auto &el : t)                                                         \
      op el;                                                                   \
    return ret;                                                                \
  }

// certain operators make sense to be bundled together
// + and +=, - and -=, ...
// this macro generates them together
#define IVL_OPS_GENERATOR2(op)                                                 \
  IVL_OPS_GENERATOR_ASSIGNMENT(op## =)                                         \
  IVL_OPS_GENERATOR_REGULAR(op)

// TODO: rename
#define IVL_OPS_FULL_GENERATOR2(op, name)                                      \
  namespace name##_impl {                                                      \
    IVL_OPS_GENERATOR2(op);                                                    \
  }                                                                            \
  namespace name {                                                             \
  using ::ivl::pointwise::name##_impl::operator op##=;                         \
  using ::ivl::pointwise::name##_impl::operator op;                            \
  }                                                                            \
  namespace all {                                                              \
  using namespace ::ivl::pointwise::name;                                      \
  }                                                                            \
  using namespace name; // this allows for `using ivl::pointwise::operator+;`

#define IVL_OPS_FULL_GENERATOR_UNARY_PRE(op, name)                             \
  namespace name##_impl {                                                      \
    IVL_OPS_GENERATOR_PRE(op);                                                 \
  }                                                                            \
  namespace name {                                                             \
  using ::ivl::pointwise::name##_impl::operator op;                            \
  }                                                                            \
  namespace all {                                                              \
  using namespace ::ivl::pointwise::name;                                      \
  }                                                                            \
  using namespace name; // this allows for `using ivl::pointwise::operator+;`

#define IVL_OPS_FULL_GENERATOR_UNARY_POST(op, name)                            \
  namespace name##_impl {                                                      \
    IVL_OPS_GENERATOR_UNARY_POST(op);                                          \
  }                                                                            \
  namespace name {                                                             \
  using ::ivl::pointwise::name##_impl::operator op;                            \
  }                                                                            \
  namespace all {                                                              \
  using namespace ::ivl::pointwise::name;                                      \
  }                                                                            \
  using namespace name; // this allows for `using ivl::pointwise::operator+;`

// looking at https://en.cppreference.com/w/cpp/language/operator_precedence
// [2] (incomplete): ++(post), --(post)
IVL_OPS_FULL_GENERATOR_UNARY_POST(++, postinc);
IVL_OPS_FULL_GENERATOR_UNARY_POST(--, postdec);
// [3] (incomplete): ++(pre), --(pre), -(unary)
IVL_OPS_FULL_GENERATOR_UNARY_POST(++, preinc);
IVL_OPS_FULL_GENERATOR_UNARY_POST(--, predec);
IVL_OPS_FULL_GENERATOR_UNARY_POST(--, neg);
// [5]: *, /, %
IVL_OPS_FULL_GENERATOR2(*, mul);
IVL_OPS_FULL_GENERATOR2(/, div);
IVL_OPS_FULL_GENERATOR2(%, mod);
// [6]: +, -
IVL_OPS_FULL_GENERATOR2(+, add);
IVL_OPS_FULL_GENERATOR2(-, sub);
// [7]: <<, >>
IVL_OPS_FULL_GENERATOR2(<<, leftshift);
IVL_OPS_FULL_GENERATOR2(>>, rightshift);
// [11]: &
IVL_OPS_FULL_GENERATOR2(&, bitwiseand);
// [12]: ^
IVL_OPS_FULL_GENERATOR2(^, bitwisexor);
// [13]: |
IVL_OPS_FULL_GENERATOR2(|, bitwiseor);

namespace arithmetic {
using namespace ::ivl::pointwise::add;
using namespace ::ivl::pointwise::sub;
using namespace ::ivl::pointwise::mul;
using namespace ::ivl::pointwise::div;
using namespace ::ivl::pointwise::mod;
using namespace ::ivl::pointwise::preinc;
using namespace ::ivl::pointwise::predec;
using namespace ::ivl::pointwise::postinc;
using namespace ::ivl::pointwise::postdec;
using namespace ::ivl::pointwise::neg;
} // namespace arithmetic

namespace bitwise {
using namespace ::ivl::pointwise::bitwiseand;
using namespace ::ivl::pointwise::bitwiseor;
using namespace ::ivl::pointwise::bitwisexor;
using namespace ::ivl::pointwise::leftshift;
using namespace ::ivl::pointwise::rightshift;
} // namespace bitwise

// TODO: add utility namespaces (all, arithmetic, ...)
// all still missing
// no longer missing

namespace dbg {

template <typename T>
std::enable_if<is_container_v<T>, std::ostream &>::type
operator<<(std::ostream &os, const T &cont) {
  for (auto &&el : cont)
    os << el << " ";
  return os;
}

} // namespace dbg

using namespace dbg;

#undef IVL_OPS_GENERATOR_ASSIGNMENT
#undef IVL_OPS_GENERATOR_REGULAR
#undef IVL_OPS_GENERATOR2
#undef IVL_OPS_FULL_GENERATOR2

} // namespace ivl::pointwise
