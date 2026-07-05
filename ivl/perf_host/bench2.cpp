#include <ivl/linux/utility>
#include <ivl/timer/timer>
#include <memory>
#include <print>
#include <vector>

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

void dump_status() {
namespace sys = ivl::linux::throwing_syscalls;
  ivl::linux::owned_file_descriptor fd{sys::open("/proc/self/status", O_RDONLY, 0)};
  constexpr std::size_t LEN = 32ULL << 12;
  char buf[LEN];
  while (auto len = sys::read(fd.get(), buf, sizeof(buf))) {
    std::print("{}", std::string_view(buf, len));
    // std::size_t off = 0;
    // while (off < len) {
    //   off += sys::write(1, buf + off, len - off);
    // }
  }
}

int ivl_main() {
  dump_status();
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
  dump_status();
  return 0;
}
