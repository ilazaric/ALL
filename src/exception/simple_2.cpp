#include <cstdio>

struct Des {
  ~Des() { puts("destructor"); }
};

void thrower();

void catch_float_cb() { throw 123; }
void catch_float();
void catch_int_cb() { catch_float(); }
void catch_int();

int main() {

  catch_int();

  // try {
  //   Des des0;
  //   try {
  //     Des des;
  //     thrower();
  //   } catch (float) {}
  // } catch (int) {}
  
}
