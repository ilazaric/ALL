using el = int;

struct S {
  el i;
  bool pred() { return i != 42; }
};

void do_something();

void foo(S& s) {
  [[assume(!s.pred())]];
  if (s.pred()) do_something();
}
