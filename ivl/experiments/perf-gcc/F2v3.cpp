#define _GLIBCXX_DEBUG
#include <array> // just for the debug stuff, not std::array

// IVL disable_ivl_main_handler()
// IVL add_compiler_flags("-fconstexpr-ops-limit=1000000000000")

namespace std
{
  __attribute__((__always_inline__,__visibility__("default")))
  inline void
  __glibcxx_assert_fail()
  { }
}

# define __glibcxx_assert_rel(cond)					\
  do {									\
    if (std::__is_constant_evaluated() && !bool(cond))			\
      std::__glibcxx_assert_fail();					\
  } while (false)

# define __glibcxx_assert_dbg(cond)					\
  do {									\
    if (__builtin_expect(!bool(cond), false))				\
      _GLIBCXX_ASSERT_FAIL(cond);					\
  } while (false)

// 3.1574s, 22173301ops
#define BASELINE

// 3.1572s, 22173301ops
#define SEMICOLON ;

// 4.4912s, 35568562ops
#define STDLIB_REL __glibcxx_assert_rel(n < this->size());

// 5.0498s, 38438969ops
#define STDLIB_DBG __glibcxx_check_subscript(n);

// 5.0606s, 38438969ops
#define STDLIB_DBG_V2                                                   \
  do {									\
    if (__builtin_expect(!bool(n < this->size()), false))               \
      _GLIBCXX_ASSERT_FAIL(cond);					\
  } while (false);

// 4.991s, 37482165ops
#define STDLIB_DBG_V3                                                   \
  if (__builtin_expect(!bool(n < this->size()), false))                 \
    _GLIBCXX_ASSERT_FAIL(cond);

// 4.9898s, 37482165ops
#define STDLIB_DBG_V4                                                   \
  if (__builtin_expect(n >= this->size(), false))                       \
    _GLIBCXX_ASSERT_FAIL(cond);

// 4.0894s, 30784537ops
#define STDLIB_DBG_V5                                                   \
  if (__builtin_expect(n >= N, false))                                  \
    _GLIBCXX_ASSERT_FAIL(cond);

// 3.502s, 27914125ops
#define STDLIB_DBG_V6                                                   \
  if (n >= N)                                                           \
    _GLIBCXX_ASSERT_FAIL(cond);

// 3.385s, 26957321ops
#define IVL_CONSTEVAL if not consteval { STDLIB_DBG }

// 3.486s, 27914125ops
#define IVL_ALWAYS if (n >= N) { STDLIB_DBG }

// 4.3056s, 34611753ops
#define IVL_ALWAYS_SIZE if (n >= this->size()) { STDLIB_DBG }

// 4.2978s, 34611753ops
#define IVL_ALWAYS_COND if (!bool(n <= this->size())) { STDLIB_DBG }

// 3.4878s, 27914125ops
#define IVL_BUILTIN if (!__builtin_is_constant_evaluated()) { STDLIB_DBG }

template<typename T, unsigned N>
struct array {
  T elems[N];

  constexpr unsigned size() const { return N; }

  constexpr T& operator[](unsigned n) {
    CHOICE
    return elems[n];
  }
};

constexpr unsigned N = 400;

constexpr auto storage = [] {
  array<array<unsigned, N>, N> out{};
  out[0][0] = 1;
  for (unsigned i = 1; i < N; ++i) out[i][0] = out[0][i] = 1;
  for (unsigned i = 1; i < N; ++i)
    for (unsigned j = 1; j < N; ++j) out[i][j] = out[i - 1][j] + out[i][j - 1];
  return out;
}();

int main() {
  array<unsigned, 5> a;
  a[10];
}
