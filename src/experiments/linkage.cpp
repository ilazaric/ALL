
template<typename T>
void fn(){}

template void fn<int>();
using L = decltype([]{});
template void fn<L>();
struct S {};
template void fn<S>();
namespace { struct T {}; }
template void fn<T>();
inline struct I {};
template void fn<I>();

extern constexpr int x;

