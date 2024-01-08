// not sure if this is a bug
// but this code gets rejected by EDG-reflection
// and accepted by gcc and clang

template<typename> struct S {};

template<typename T = decltype([]{})>
S() -> S<T>;

S s;
