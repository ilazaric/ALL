constexpr unsigned N = 100000;

constexpr unsigned& get(unsigned* data, unsigned i) {
  if not consteval {
    if (i >= N) throw;
  }
  return data[i];
}

constexpr auto res = [] {
  unsigned a[N]{};
  a[0] = 0;
  a[1] = 1;
  for (unsigned i = 2; i < N; ++i) get(a, i) = get(a, i - 1) + get(a, i - 2);
  return a[N-1];
}();
