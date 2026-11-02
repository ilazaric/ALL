namespace std::meta {
using info = decltype(^^int);
}

struct refl_span {
  const std::meta::info* data;
  int length;
  constexpr auto begin() const { return data; }
  constexpr auto end() const { return data + length; }
};

constexpr std::meta::info els[] = {^^int, ^^float, ^^char};
constexpr auto Ers = refl_span{els, 3};

template<typename = void>
constexpr int foo(int x) {
  template for (constexpr auto er : Ers) {}
  return x;
}

int main() {
  int x = 1;
  (void)foo(x);
}

// IVL add_compiler_flags("-fsanitize=undefined")
// IVL disable_ivl_main_handler()

// compile with:
// g++ -std=c++26 -freflection ubsan-bug.cpp -fsyntax-only -fsanitize=undefined
