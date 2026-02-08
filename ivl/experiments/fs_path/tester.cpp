#include "evil_lexically_normal.hpp"
#include <iostream>

void check_equal(const std::filesystem::path& p) {
  auto lhs = p.lexically_normal();
  auto rhs = impl::lexically_normal(p);
  if (lhs == rhs && lhs.native() == rhs.native()) return;
  std::cout << "FOUND MISMATCH\n";
  std::cout << "INPUT   : " << p << " -- " << p.native() << std::endl;
  std::cout << "EXPECTED: " << lhs << " -- " << lhs.native() << std::endl;
  std::cout << "PRODUCED: " << rhs << " -- " << rhs.native() << std::endl;
  exit(1);
}

int main() {
  check_equal("/home/dotdot/some/dot/really/dotdot/dot/dotdot/long/path/");
  check_equal("");
  check_equal("/");
  check_equal("../../..");
  check_equal("/..");

  check_equal("a/.");
  check_equal("/.");
  check_equal("/...//.");
  check_equal(".");
  check_equal("/a/a/..");
  // exit(0);

  srand(42);

  for (size_t i = 0; i < 10000000; ++i) {
    std::string s(rand() % 20, '\0');
    for (auto& c : s) {
      auto sel = rand() % 3;
      if (sel == 0) c = '.';
      else if (sel == 1) c = '/';
      else c = 'a' + rand() % 26;
    }
    check_equal(s);
  }
}
