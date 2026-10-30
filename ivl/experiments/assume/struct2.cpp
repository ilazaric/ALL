struct S {
  bool x;
};

void do_something();

void foo(S& s) {
  if (s.x) do_something();
}

void bar(S& s) {
  [[assume(!s.x)]];
  foo(s);
}
