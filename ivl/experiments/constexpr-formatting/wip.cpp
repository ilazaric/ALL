#include <format>
#include <string_view>
#include <string>
#include <vector>

consteval {
  (void)std::format("{}", std::vector<int>{});
}
