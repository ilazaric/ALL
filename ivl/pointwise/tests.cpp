#include "pointwise"

#include <vector>
#include <array>
#include <cassert>

bool test_throw(){
  using ivl::pointwise::operator+;
  std::vector<int> a{1, 2, 3};
  std::vector<int> b{1, 2};
  try {
    a + b;
    return false;
  } catch (const ivl::pointwise::PointwiseException&){
    return true;
  }
}

bool test_correct(){
  using ivl::pointwise::operator*;
  std::array<int, 4> a{1, 2, 3, 4};
  std::array<int, 4> b{2, 3, 4, 5};
  auto c = a * b;
  return c == std::array<int, 4>{2, 6, 12, 20};
}

int main(){

  assert(test_throw());
  assert(test_correct());
  
  return 0;
}
