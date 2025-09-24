#include <ivl/alloc/mmap_fixed_storage>
#include <ivl/alloc/small_ptr_allocator>
#include <vector>

template <std::size_t N>
struct DynArray {
  std::vector<char> vec;
  DynArray() : vec(N) {}
  constexpr char*       data() { return vec.data(); }
  constexpr std::size_t size() { return N; }
};

struct AllocTraits {
  // inline static DynArray<(128ULL<<20)> storage;
  inline static ivl::alloc::MmapFixedStorage<0x0000'0300'0000'0000, (128ULL << 20)> storage;
  static constexpr std::size_t                                                      segment_tree_chunk_size = 64;
  static constexpr std::size_t                                                      free_list_limit         = 256;
  static constexpr std::size_t                                                      free_list_steal_coef    = 64;
};

struct AllocDynTraits {
  inline static DynArray<(128ULL << 20)> storage;
  // inline static ivl::alloc::MmapFixedStorage<0x0000'0300'0000'0000, (128ULL<<20)> storage;
  static constexpr std::size_t segment_tree_chunk_size = 64;
  static constexpr std::size_t free_list_limit         = 256;
  static constexpr std::size_t free_list_steal_coef    = 64;
};

template <typename T>
using Alloc = ivl::alloc::SmallPtrAllocator<T, AllocTraits>;

template <typename T>
using DynAlloc = ivl::alloc::SmallPtrAllocator<T, AllocDynTraits>;

void consume(int);

void deref_fancy(Alloc<int>::pointer ptr) { consume(*ptr); }

void deref_fancy_dyn(DynAlloc<int>::pointer ptr) { consume(*ptr); }

void deref_raw(int* ptr) { consume(*ptr); }

void deref_fancy2(Alloc<int>::pointer ptr1, Alloc<int>::pointer ptr2) {
  consume(*ptr1);
  consume(*ptr2);
}

void deref_raw2(int* ptr1, int* ptr2) {
  consume(*ptr1);
  consume(*ptr2);
}

template <typename P>
int accum(P ptr, std::size_t len) {
  int res = 0;
  for (std::size_t i = 0; i < len; ++i)
    res += ptr[i];
  return res;
}

template int accum<int*>(int*, std::size_t);
template int accum<Alloc<int>::pointer>(Alloc<int>::pointer, std::size_t);
