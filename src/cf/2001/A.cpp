#include <ivl/io/conversion>
#include <ivl/io/stlutils>
#include <ivl/logger>
#include <map>
#include <ranges>

void one() {
  std::vector<int> vec{cin};

  LOG(vec);

  std::map<int, int> bla;
  for (auto x : vec)
    ++bla[x];
  int maks = 0;
  for (auto [x, y] : bla)
    maks = std::max(maks, y);
  std::cout << vec.size() - maks << std::endl;
}

int main() {
  for (int t{cin}; t--;) {
    one();
  }

  return 0;
}
