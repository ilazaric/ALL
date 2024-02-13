#include <vector>

#include "small_ptr_allocator.hpp"

#include <ivl/logger/logger.hpp>
using namespace ivl::logger::default_logger;

template <typename T>
using SmallVec = std::vector<
    T, ivl::alloc::SmallPtrEnv<0xAFED'0000'0000'0000ULL>::Allocator<T>>;

static_assert(sizeof(SmallVec<int>) == 12);
static_assert(sizeof(std::vector<int>) == 24);

int main() {
  SmallVec<int> vec;
  vec.push_back(123);
}
