#include <ivl/format>
#include <source_location>

template <typename... Args>
struct panic {
  [[noreturn]] constexpr explicit panic(
    Args&&... args, std::string_view header = "!!! PANIC !!!",
    std::source_location loc = std::source_location::current()
  ) {
    if consteval {
      throw 123;
    } else {
      throw 456;
    }
  }
};

template <typename... Args>
panic(auto&&, Args&&...) -> panic<ivl::fmt::format_string<Args...>, Args...>;
template <typename = void>
panic() -> panic<>;

struct foo_todo {
  [[noreturn]] constexpr explicit foo_todo(std::source_location loc = std::source_location::current()) {
    panic<ivl::fmt::format_string<>>("TODO not implemented", "!!! TODO PANIC !!!", loc);
  }
};

// IVL disable_ivl_main_handler()

int main() {}
