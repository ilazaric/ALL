
struct S {
  int a;
  int b;
  int c[1022];
};

void consume(auto);

void useA(S& s) {
  consume(s.a);
  consume(42);
}

void useB(S& s) {
  consume(s.b);
  consume(42);
}

void useC(S& s) {
  consume(s.c[1021]);
  consume(42);
}
