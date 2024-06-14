#include <iostream>
#include <vector>
#include <algorithm>

int main(){

  int n; std::cin >> n;
  std::vector<int> p(n);
  for (auto& x : p){std::cin >> x; --x;}

  std::vector<std::pair<int, int>> out;

  if (n % 2 == 1){
    auto it = std::ranges::find(p, n-1);
    for (int loc = it - p.begin();
         loc != n-1;
         ++loc)
      out.emplace_back(loc+1, loc+2);
    --n;
    p.erase(it);
  }

  auto simulate = [&](int a, int c){
    if (a % 2 == 0){
      a += c;
      if (a >= n) a = n-1-(a-n);
      return a;
    } else {
      a -= c;
      if (a < 0) a = -1-a;
      return a;
    }
  };

  auto dist = [&](int a, int b){
    if (a > b) std::swap(a, b);
    if (a+1 == b) return 0;
    
    int rem = 0;

    if (a % 2 == 1){
      int x = a + 1;
      a = simulate(a, x);
      b = simulate(b, x);
      rem += x;
    }

    if (b % 2 == 0){
      int x = n-b;
      a = simulate(a, x);
      b = simulate(b, x);
      rem += x;
    }

    return (b-a)/2;
  };

  std::vector<int> q(n);
  for (int i = 0; i < n; ++i)
    q[p[i]] = n-1-i;

  auto solve_cycle = [&](std::vector<int> c){
    if (c.size() == 1) return;
    auto cpy = c;
    std::ranges::sort(cpy);
    for (int i = 0; i+1 < cpy.size(); ++i)
  };
  
  for (int i = 0; i < n-1; ++i){
    out.emplace_back(i % 2 + 1, n - i % 2);
  }

  std::cout << out.size() << std::endl;
  for (auto [l, r] : out)
    std::cout << l << " " << r << std::endl;

  return 0;
}
