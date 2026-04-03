// this is included by all syscall semantic headers, to emit helpers

#ifndef X_NAMESPACE
#error "X_NAMESPACE must be defined to include this"
#endif

namespace X_NAMESPACE {
struct semantic_t {};
inline constexpr semantic_t semantic;

#define X_PARAMS0()
#define X_PARAMS1(t1, a1) t1 a1
#define X_PARAMS2(t1, a1, ...) X_PARAMS1(t1, a1), X_PARAMS1(__VA_ARGS__)
#define X_PARAMS3(t1, a1, ...) X_PARAMS1(t1, a1), X_PARAMS2(__VA_ARGS__)
#define X_PARAMS4(t1, a1, ...) X_PARAMS1(t1, a1), X_PARAMS3(__VA_ARGS__)
#define X_PARAMS5(t1, a1, ...) X_PARAMS1(t1, a1), X_PARAMS4(__VA_ARGS__)
#define X_PARAMS6(t1, a1, ...) X_PARAMS1(t1, a1), X_PARAMS5(__VA_ARGS__)

#define X_CARGS0()
#define X_CARGS1(t1, a1) a1
#define X_CARGS2(t1, a1, ...) a1, X_CARGS1(__VA_ARGS__)
#define X_CARGS3(t1, a1, ...) a1, X_CARGS2(__VA_ARGS__)
#define X_CARGS4(t1, a1, ...) a1, X_CARGS3(__VA_ARGS__)
#define X_CARGS5(t1, a1, ...) a1, X_CARGS4(__VA_ARGS__)
#define X_CARGS6(t1, a1, ...) a1, X_CARGS5(__VA_ARGS__)

#define X(N, name, ...)                                                                                                \
  inline decltype(auto) name(semantic_t __VA_OPT__(, ) X_PARAMS##N(__VA_ARGS__)) {                                     \
    return ::X_NAMESPACE::name(X_CARGS##N(__VA_ARGS__));                                                               \
  }

#include <ivl/linux/syscall_arguments_X>
#include <ivl/linux/syscall_arguments_fat_clone3_X>

#undef X

#undef X_PARAMS0
#undef X_PARAMS1
#undef X_PARAMS2
#undef X_PARAMS3
#undef X_PARAMS4
#undef X_PARAMS5
#undef X_PARAMS6

#undef X_CARGS0
#undef X_CARGS1
#undef X_CARGS2
#undef X_CARGS3
#undef X_CARGS4
#undef X_CARGS5
#undef X_CARGS6
} // namespace X_NAMESPACE
