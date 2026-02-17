// #define private public
// #define protected public
#include <meta>

#include <cassert>
#include <format>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include <functional>

#include <print>

// minimized example of libstdc++ constexpr std::set issues
struct NodeBase {
  int data;
  NodeBase* base_ptr() { return this; }
};

struct Node : NodeBase {};

consteval {
  if (0) {
    std::allocator<Node> alloc;

    Node* node_ptr = alloc.allocate(1);
    NodeBase* base_ptr = node_ptr->base_ptr();
    Node* node_ptr_copy = static_cast<Node*>(base_ptr);

    assert(node_ptr == node_ptr_copy);
    alloc.deallocate(node_ptr_copy, 1);

    throw;
  }
}
// ~minimal example

// trying out constexpr formatting

constexpr std::string format_mountain() {
  std::string output;
  std::format_to(std::back_inserter(output), "---------------------------\n");
  std::format_to(std::back_inserter(output), "FORMATTING OF VARIOUS TYPES\n");
  std::format_to(std::back_inserter(output), "---------------------------\n");
  std::format_to(std::back_inserter(output), "\n");

  auto format_into = [&](std::string_view expr_str, auto&& expr) {
    std::format_to(std::back_inserter(output), "EXPRESSION {:?} FORMATS INTO: {}\n", expr_str, expr);
  };

#define FMT(...) format_into(#__VA_ARGS__, __VA_ARGS__)
#define NO(...) 0

  FMT(true);
  FMT(false);
  FMT(123);
  // FMT(-4.5f);
  FMT(678ULL);

#define EXPAND_STRS(...)                                                                                               \
  do {                                                                                                                 \
    FMT(__VA_ARGS__);                                                                                                  \
    FMT(std::string_view(__VA_ARGS__));                                                                                \
    FMT(std::string(__VA_ARGS__));                                                                                     \
  } while (0)
  EXPAND_STRS("");
  EXPAND_STRS("short");
  EXPAND_STRS("long long long long long long long long long long long long long long long long long long");
  EXPAND_STRS(
    "REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG "
    "REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG "
    "REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG "
    "REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG "
    "REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG "
    "REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG "
    "REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG "
    "REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG "
    "REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG "
    "REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG "
    "REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG "
    "REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG "
    "REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG "
    "REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG "
    "REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG "
    "REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG REALLY LONG "
  );

#define EXPAND_CNTRS(...)                                                                                              \
  do {                                                                                                                 \
    FMT(std::vector{__VA_ARGS__});                                                                                     \
    NO(std::set{__VA_ARGS__});                                          \
  } while (0)
  EXPAND_CNTRS(1, 2, 3, 4);
  // EXPAND_CNTRS(4.1f, 5.2f, 6.3f);
  EXPAND_CNTRS(std::vector{1, 2}, std::vector{3}, std::vector{4, 5, 6});

  NO(std::map<int, int>{});
  NO(std::map<int, int>{{1, 1}, {2, 2}, {3, 3}});
  NO(
    std::map<int, std::map<std::string, std::vector<int>>>{
      {1,
       {
         {"a", {2, 3, 4}},
         {"b", {5}},
         {"c", {6, 7}},
       }},
      {2,
       {
         {"x", {}},
         {"y", {-1, -1, -1}},
         {"z", {0}},
       }},
    }
  );

  return output;
}

constexpr std::string_view constexpr_output = std::define_static_string(format_mountain());

int main() {
  std::print("{}", constexpr_output);
}
