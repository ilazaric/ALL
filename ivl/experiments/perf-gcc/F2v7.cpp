// IVL disable_ivl_main_handler()
// IVL add_compiler_flags("-fconstexpr-ops-limit=1000000000000")

template<typename T, unsigned N>
struct array {
  T elems[N];
  constexpr unsigned size() const { return N; }
  constexpr T& operator[](unsigned n) { return elems[n]; }
  constexpr const T& operator[](unsigned n) const { return elems[n]; }
};

constexpr unsigned N = 300;

constexpr auto storage = [] //(int x = 1)
 {
  array<array<unsigned, N>, N> out{};
  out[0][0] = 1;
  for (unsigned i = 1; i < N; ++i) out[i][0] = out[0][i] = 1;
  for (unsigned i = 1; i < N; ++i)
    for (unsigned j = 1; j < N; ++j) out[i][j] = out[i - 1][j] + out[i][j - 1];
  if not consteval { throw; }
  return out;
}();

static_assert(storage[2][2] == 6);
