#include <iostream>
#include <string>

void solve() {
  std::string s, t;
  std::cin >> s >> t;

  int s_cnt[26]{};
  int t_cnt[26]{};

  for (auto c : s) ++s_cnt[c-'a'];
  for (auto c : t) ++t_cnt[c-'a'];

  for (int i = 0; i < 26; ++i)
    if (s_cnt[i] > t_cnt[i]) {
      std::cout << "Impossible\n";
      return;
    }

  auto it = s.begin();
  for (int i = 0; i < 26; ++i) {
    while (it != s.end() && *it-'a' <= i)
      std::cout << *it++;
    for (int j = 0; j < t_cnt[i] - s_cnt[i]; ++j)
      std::cout << (char)('a' + i);
  }
  std::cout << std::endl;
}

int main() {
  int t;
  std::cin >> t;
  for (int i = 0; i < t; ++i)
    solve();
}
