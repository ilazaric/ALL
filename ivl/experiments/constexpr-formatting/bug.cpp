struct S {};

constexpr S tail(S x) {
  throw 123;
  return x;
}

constexpr inline void head() { tail(S{}); }

consteval {
  try {
    head();
    asm("");
  } catch (int) {
  }

  try {
    head();
    asm("");
  } catch (int) {
  }
}
