#warning "TODO, think this was just test input"

#include <ivl/format>
#include <vector>

int main() {
  std::vector<int> vec{1,2,3};
  ivl::fmt::println("{}", vec);
}
