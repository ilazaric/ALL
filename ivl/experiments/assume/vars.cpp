using el = int;

struct S {
  el i;
  bool pred() { return i != 42; }
};

void do_something();

void foo_builtin(S& s) {
  if (s.pred()) __builtin_unreachable();
  if (s.pred()) do_something();
}

void foo_attribute(S& s) {
  [[assume(!s.pred())]];
  if (s.pred()) do_something();
}
