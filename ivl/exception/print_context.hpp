#include <ivl/format>
#include <ivl/utility/scope_exit>
#include <exception>

// TODO: source location
#define EXCEPTION_PRINT_CONTEXT(...)                                                                                   \
  ::ivl::util::scope_exit _ {                                                                                          \
    [&, EXCEPTION_PRINT_CONTEXT_exception_count = std::uncaught_exceptions()] {                                        \
      if (std::uncaught_exceptions() > EXCEPTION_PRINT_CONTEXT_exception_count) {                                      \
        ::ivl::fmt::print(stderr, "  exception print context({}): ", EXCEPTION_PRINT_CONTEXT_exception_count);         \
        ::ivl::fmt::println(stderr, __VA_ARGS__);                                                                      \
      }                                                                                                                \
    }                                                                                                                  \
  }
