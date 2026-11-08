// {1}:
// : T
// : S
// {2}:
// : if ({3}) __builtin_unreachable()
// : [[assume(!{3})]]
// {3}:
// : t.pred()
// : t.x
// {4}:
// : t.pred()
// : t.x

struct S {
  bool x;
  bool pred() { return x; }
};

struct T : S {};

void foo({1}& t) {
  {2};
  if ({4}) __builtin_abort();
}
