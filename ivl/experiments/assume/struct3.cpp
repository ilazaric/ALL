struct S {
  bool x;
  bool pred() { return x; }
  S();
  ~S();
};

void do_something();

void fn(S s) {
  [[assume(!s.pred())]];
  if (s.pred()) do_something();
}
