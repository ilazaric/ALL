#include <experimental/meta>
#include <type_traits>
#include <concepts>

template<typename T, std::meta::info C>
concept Con = true;

static_assert(Con<int, ^int>); // passes
static_assert(Con<int, ^std::copyable>); // fails
