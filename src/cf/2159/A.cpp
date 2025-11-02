#include <ivl/io/conversion>
#include <ivl/meta>
#include <cstdint>
#include <iostream>
#include <vector>

using ivl::io::conversion::cin;

int main() {
  ivl::meta::repeat(size_t{cin}, [] {
    uint32_t n{cin};

    std::vector<uint32_t> query_arg;
    auto                  query = [&] {
      std::cout << "? " << query_arg.size();
      for (auto el : query_arg)
        std::cout << " " << el + 1;
      std::cout << std::endl;
      return uint32_t{cin};
    };

    std::vector<uint32_t> answer(2 * n, 0);

    // This loop consumes 2n queries and deduces n elements of sequence.
    // Each value is deduced exactly once.
    for (uint32_t i = 0; i < 2 * n; ++i) {
      query_arg.push_back(i);
      auto query_ans = query();
      if (query_ans == 0) continue;
      answer[i] = query_ans;
      query_arg.pop_back();
    }

    query_arg.clear();
    for (uint32_t i = 0; i < 2 * n; ++i)
      if (answer[i]) query_arg.push_back(i);

    for (uint32_t i = 0; i < 2 * n; ++i)
      if (answer[i] == 0) {
        query_arg.push_back(i);
        answer[i] = query();
        query_arg.pop_back();
      }

    std::cout << "!";
    for (auto el : answer)
      std::cout << " " << el;
    std::cout << std::endl;
  });
}
