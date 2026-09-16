// #define FMT_HEADER_ONLY
#include <fmt/format.h>
// #include <fmt/ranges.h>
#include <array>
#include <span>

int main() {
  // std::array<const char*, 4> a{};
  // fmt::format("hello world long array: {::?}", std::span<const char*>(a));
  fmt::format("hello world long array");
}
