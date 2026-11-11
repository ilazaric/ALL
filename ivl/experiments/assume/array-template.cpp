// {1}:
// : 0
// : 1

struct S {
  bool data[2];
};

bool get(S& s, int i) {
  return s.data[i];
}

void foo(S& s) {
  [[assume(!get(s, {1}))]];
  if (s.data[{1}]) __builtin_abort();
}
