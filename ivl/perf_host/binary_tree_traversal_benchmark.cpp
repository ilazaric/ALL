#include <ivl/timer/timer>
#include <memory>
#include <vector>
#include <print>

// IVL add_compiler_flags("-static")

struct node;
using tree = std::unique_ptr<node>;

struct node {
  std::size_t value;
  std::size_t count = 1;
  tree left;
  tree right;
};

void insert(tree& curr, std::size_t value) {
  if (!curr) {
    curr = std::make_unique<node>(value);
    return;
  }
  insert(curr->value > value ? curr->left : curr->right, value);
  ++curr->count;
}

std::size_t size(tree& curr) { return curr ? curr->count : 0; }

std::size_t index(tree& curr, std::size_t value) {
  if (!curr) return 0;
  if (curr->value > value) return index(curr->left, value);
  return size(curr->left) + 1 + index(curr->right, value);
}

int ivl_main() {
  srand(42);
  tree root;
  for (int i = 0; i < 100'000; ++i) insert(root, rand());
  std::vector<std::size_t> queries;
  for (int i = 0; i < 1'000'000; ++i) queries.push_back(rand());
  std::size_t acc = 0;
  auto start = ivl::timer::gettimestamp();
  for (auto el : queries) acc += index(root, el);
  auto end = ivl::timer::gettimestamp();
  std::println("duration: {}", end - start);
  std::println("evidence: {}", acc);
  return 0;
}
