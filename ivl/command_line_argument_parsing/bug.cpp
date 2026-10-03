#include <ivl/format>
#include <source_location>

#define FWD(x) std::forward<decltype(x)>(x)

#pragma IVL add_compiler_flags_tail "-lstdc++exp"

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
  constexpr operator bool() const noexcept { return true; };
};

template <typename... Args>
panic(auto&&, Args&&...) -> panic<ivl::fmt::format_string<Args...>, Args...>;
template <typename = void>
panic() -> panic<>;

template <typename... Args>
struct todo {
  [[noreturn]] constexpr explicit todo(Args&&... args, std::source_location loc = std::source_location::current()) {
    panic<Args...>(FWD(args)..., "!!! TODO PANIC !!!", loc);
  }
  constexpr operator bool() const noexcept { return true; };
};

template <>
struct todo<> {
  [[noreturn]] constexpr explicit todo(std::source_location loc = std::source_location::current()) {
    panic<ivl::fmt::format_string<>>("TODO not implemented", "!!! TODO PANIC !!!", loc);
  }
  constexpr operator bool() const noexcept { return true; };
};

template <typename... Args>
todo(auto&&, Args&&...) -> todo<ivl::fmt::format_string<Args...>, Args...>;
template <typename = void>
todo() -> todo<>;

// IVL disable_ivl_main_handler()

int main() {}
