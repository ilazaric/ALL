struct A {
  [[noreturn]] A() { throw 123; }
};

[[noreturn]] void f() { throw 456; }

struct B {
#pragma diag_suppress noreturn_function_does_return
  [[noreturn]] B() { A{}; }
#pragma diag_default noreturn_function_does_return
};

// IVL disable_ivl_main_handler()

int main() {}
