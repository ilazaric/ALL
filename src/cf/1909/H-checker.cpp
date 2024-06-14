#include <cassert>
#include <fstream>
#include <iostream>
#include <algorithm>

int main(int argc, char** argv){
  assert(argc == 3);

  std::ifstream fin(argv[1]);
  int n; fin >> n;
  std::vector<int> p(n);
  for (auto& x : p) fin >> x;

  std::ifstream fans(argv[2]);
  int k; fans >> k;
  for (int i = 0; i < k; ++i){
    int l, r; fans >> l >> r;
    for (int x = l; x < r; x += 2)
      std::swap(p[x-1], p[x]);
  }

  assert(std::ranges::is_sorted(p));
}
