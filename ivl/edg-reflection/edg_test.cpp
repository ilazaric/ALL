
template <typename>
struct S {};

template <typename T = decltype([] {})>
S() -> S<T>;

S s;
