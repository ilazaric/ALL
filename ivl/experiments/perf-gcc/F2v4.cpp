// IVL disable_ivl_main_handler()
// IVL add_compiler_flags("-fconstexpr-ops-limit=1000000000000")
// IVL add_compiler_flags("-DCHOICE=BASELINE")

#define BASELINE

#define STDLIB_DBG if (n >= N) { throw; }

#define IVL_CONSTEVAL if not consteval { STDLIB_DBG }

#define IVL_ALWAYS if (n >= N) { STDLIB_DBG }

#define IVL_ALWAYS_SIZE if (n >= this->size()) { STDLIB_DBG }

#define IVL_ALWAYS_COND if (!bool(n < this->size())) { STDLIB_DBG }

#define IVL_BUILTIN if (!__builtin_is_constant_evaluated()) { STDLIB_DBG }

#define IVL_ALWAYS_V2 if (true) if (n >= N) { STDLIB_DBG }

#define IVL_ALWAYS_EXPECT if (__builtin_expect(n >= N, false)) { STDLIB_DBG }

template<typename T, unsigned N>
struct array {
  T elems[N];

  constexpr unsigned size() const { return N; }

  constexpr T& operator[](unsigned n) {
    CHOICE
    return elems[n];
  }
};

constexpr unsigned N = 300;

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
