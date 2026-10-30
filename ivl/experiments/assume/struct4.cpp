struct S {
  bool x;
  bool pred() { return x; }
};

struct T : S {};

void do_something();

void foo_builtin(T& t) {
  if (t.pred()) __builtin_unreachable();
  if (t.pred()) do_something();
}

void foo_attribute(T& t) {
  [[assume(!t.x)]];
  if (t.x) do_something();
}
