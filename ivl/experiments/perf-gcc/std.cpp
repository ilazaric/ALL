#include <format>
#include <array>
#include <span>

int main() {
  std::array<const char*, 4> a{};
  std::format("hello world long array: {::?}", std::span<const char*>(a));
}
