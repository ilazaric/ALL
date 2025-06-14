#include <ranges>
// #include <ivl/stl/vector>
// #include <ivl/alloc/small_ptr_allocator.hpp>
#include <ivl/io/conversion>
// #include <ivl/logger>
#include <ivl/alloc/mmap_fixed_storage.hpp>
#include <ivl/io/stlutils.hpp>
#include <vector>

// template<std::size_t N>
// struct DynArray {
//   std::vector<char> vec;
//   DynArray() : vec(N){}
//   constexpr char* data(){return vec.data();}
//   constexpr std::size_t size(){return N;}
// };

// struct AllocTraits {
//   // inline static DynArray<(128ULL<<20)> storage;
//   // inline static std::array<char, (25ULL<<20)> storage;
//   inline static ivl::alloc::MmapFixedStorage<0x0000'0300'0000'0000, (128ULL<<20)> storage;
//   static constexpr std::size_t segment_tree_chunk_size = 64;
//   static constexpr std::size_t free_list_limit = 256;
//   static constexpr std::size_t free_list_steal_coef = 64;
// };

// template<typename T>
// struct ivl::alloc::GlobalAlloc : ivl::alloc::SmallPtrAllocator<T, AllocTraits> {};

template <typename T>
struct ivl::alloc::GlobalAlloc : std::allocator<T> {};

int main() {
  // LOG(sizeof(ivl::alloc::spa_detail::Allocator<AllocTraits>::segment_tree_type));
  // exit(1);
  for (auto ti : std::views::iota(0, int {cin})) {
    std::uint32_t              n {cin}, b {cin}, x {cin};
    std::vector<std::uint32_t> cs(n);
    std::cin >> ivl::io::Elems {cs};

    auto eval = [&](std::uint32_t k) {
      std::int64_t out = -(std::int64_t(k - 1)) * x;
      for (auto c : cs) {
        std::int64_t sz1  = c / k;
        std::int64_t sz2  = sz1 + 1;
        std::int64_t cnt2 = c % k;
        std::int64_t cnt1 = k - cnt2;
        out += b * cnt1 * cnt2 * sz1 * sz2;
        out += b * cnt1 * (cnt1 - 1) / 2 * sz1 * sz1;
        out += b * cnt2 * (cnt2 - 1) / 2 * sz2 * sz2;
      }
      return out;
    };

    std::uint32_t lo = 1, hi = 2e5, mid1, mid2;
    while (hi > lo + 5) {
      mid1 = (lo * 2 + hi) / 3;
      mid2 = (lo + hi * 2) / 3;
      if (eval(mid1) > eval(mid2)) {
        hi = mid2;
      } else {
        lo = mid1;
      }
    }

    std::cout << std::ranges::max(std::views::iota(lo, hi + 1) | std::views::transform(eval))
              << std::endl;
  }
}
