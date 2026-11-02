#include <meta>

enum class E { A, B, C };

template<typename T>
struct my_span {
  T* data;
  size_t length;
  // constexpr my_span(std::span<T> s) : data(s.data()), length(s.size()) {}
  constexpr my_span(T* data, size_t length) : data(data), length(length) {}
  constexpr const T* begin() const { return data; }
  constexpr const T* end() const { return data + length; }
};

constexpr std::meta::info els[] = {^^int, ^^float, ^^char};
constexpr auto Ers = my_span<const std::meta::info>(els, 3);

template<typename = void>
constexpr E foo(E e) {
  template for (constexpr auto er : Ers) {}
  return E::C;
}

// IVL add_compiler_flags("-fsanitize=undefined")
// IVL disable_ivl_main_handler()

int main() {
  E e = E::B;
  (void)foo(e);
}

// compile with:
// g++ -std=c++26 -freflection ubsan-bug.cpp -fsyntax-only -fsanitize=undefined
