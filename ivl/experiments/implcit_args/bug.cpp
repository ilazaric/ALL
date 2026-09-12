#include "implicit2"

// IVL add_compiler_flags("-Wno-non-template-friend -Wsfinae-incomplete=0")

int main() {
  return *implicit_name<bool>().value_ptr;
  // return implicit_flag("foo");
}
