struct T {
  int x;
  constexpr T(int x) : x(x){}
  constexpr T(const T& x) : x(x.x){}
};

struct S {
  T x;
  T y = x;
};

constexpr bool fn(){
  T t{12};
  S s{.x = t};
  return s.x.x == s.y.x;
}

static_assert(fn());
