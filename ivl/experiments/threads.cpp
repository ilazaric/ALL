#include <thread>
#include <vector>

#include <ivl/logger>

long long x = 0;

void task() {
  for (int i = 0; i < 1000000; ++i)
    ++x;
}

int main() {
  std::vector<std::thread> vec;
  for (int i = 0; i < 2; ++i)
    vec.emplace_back(task);
  // LOG(vec.size());
  for (auto& t : vec)
    t.join();
  LOG(x);
}
