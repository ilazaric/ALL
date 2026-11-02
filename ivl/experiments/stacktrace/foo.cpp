#include <print>
#include <ranges>
#include <stacktrace>

// IVL add_compiler_flags("-fno-omit-frame-pointer")

uintptr_t deref(uintptr_t v) { return *reinterpret_cast<uintptr_t*>(v); }

// blocks inlining and tail call optimization
#define BLOCK [[gnu::noinline]] [[gnu::optimize("no-optimize-sibling-calls")]]

BLOCK void print_rbp(uintptr_t v) { std::println("rbp={:x} -> orig={:x} rbp={:x}", v, deref(v + 8) - 1, deref(v)); }

BLOCK void print_rsp(uintptr_t v) { std::println("rsp={:x}", v); }

struct frame_iterator {
  using iterator_category = std::forward_iterator_tag;
  using difference_type = ptrdiff_t;
  using value_type = uintptr_t;

  uintptr_t rbp;

  uintptr_t operator*() const { return deref(rbp + 8) - 1; }

  frame_iterator& operator++() {
    rbp = deref(rbp);
    return *this;
  }

  frame_iterator operator++(int) {
    auto copy = *this;
    ++*this;
    return copy;
  }

  bool operator==(const frame_iterator&) const = default;
};

struct frame_range : std::ranges::view_interface<frame_range> {
  uintptr_t rbp;
  BLOCK frame_range() {
    register uintptr_t rbp_ asm("rbp");
    rbp = rbp_;
  }
  explicit frame_range(uintptr_t rbp) : rbp(rbp) {}
  auto begin() const { return frame_iterator{rbp}; }
  auto end() const { return frame_iterator{0}; }
};

template<>
constexpr bool std::ranges::enable_borrowed_range<frame_range> = true;

static_assert(std::ranges::forward_range<frame_range>);
static_assert(std::ranges::borrowed_range<frame_range>);
static_assert(std::ranges::view<frame_range>);

template<int N>
BLOCK void foo() {
  if constexpr (N == 0) {
    std::println(
      "{::x}",
      std::stacktrace::current() | std::views::transform(&std::stacktrace_entry::native_handle) | std::views::drop(1)
    );
    std::println("{::x}", frame_range() | std::views::drop(1));
  } else {
    foo<N - 1>();
  }
}

int main() { foo<10>(); }
