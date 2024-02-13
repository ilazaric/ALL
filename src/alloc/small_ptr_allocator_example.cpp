#include <vector>
#include <map>
#include <memory>

#include "small_ptr_allocator.hpp"

#include <ivl/logger>

struct Traits {
  inline static std::array<char, (1ULL<<20)> storage;
  static constexpr std::size_t segment_tree_chunk_size = 64;
  static constexpr std::size_t free_list_limit = 256;
  static constexpr std::size_t free_list_steal_coef = 64;
};

using Alloc = ivl::alloc::SmallPtrAllocator<char, Traits>;

ivl::alloc::SmallPtrAllocator<float, Traits> alloc;

int main(){
  LOG(123);

  {int stackptr;
    LOG(&stackptr);
  }
  
  {
    auto p = alloc.allocate(20);
    alloc.deallocate(p, 20);
  }
  
  std::vector<int, ivl::alloc::SmallPtrAllocator<int, Traits>> vec;
  LOG(sizeof(vec));
  for (int i = 0; i < 1000; ++i)
    vec.push_back(i);

  int acc = 0;
  for (auto x : vec)
    acc += x;
  LOG(acc);

  using Map = std::map<int, int, std::less<int>, Alloc::rebind<std::pair<const int, int>>::other>;

  auto m = Alloc::rebind<Map>::other::allocate(1);
  std::construct_at(&*m);

  for (int i = 0; i < 1000; ++i)
    (*m)[i] = i;

  std::destroy_at(&*m);
  Alloc::rebind<Map>::other::deallocate(m, 1);
}

