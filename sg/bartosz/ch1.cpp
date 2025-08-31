#include <memory>

#include <ivl/logger>

auto id = [](auto&& x) { return std::forward<decltype(x)>(x); };

auto compose = [](auto&& f, auto&& g) { return [&](auto&& x) { return f(g(std::forward<decltype(x)>(x))); }; };

bool compare(auto&& f, auto&& g) {
  for (int i = 0; i < 100; ++i)
    if (f(i) != g(i)) return false;
  return true;
}

int fn(int x) { return x + 3; }

int main() {
  LOG(compare(fn, compose(fn, id)));
  LOG(compare(fn, compose(id, fn)));
}
