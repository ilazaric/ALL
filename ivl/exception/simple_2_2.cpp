#include <cstdio>

void thrower() {
  throw 123;
}

struct Des2 {
  ~Des2() { puts("destructor2"); }
};

void catch_int_cb();
void catch_int() {
  Des2 dd;
  try {
    Des2 d;
    catch_int_cb();
  } catch (int) {}
}

void catch_float_cb();
void catch_float() {
  Des2 dd;
  try {
    Des2 d;
    catch_float_cb();
  } catch (float) {}
}
