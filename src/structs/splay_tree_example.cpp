#include "splay_tree.hpp"

struct S {
  int64_t x;
  int64_t sum;
  S(int64_t x) : x(x), sum(x) {}
  void refresh_state(S* left, S* right) {
    sum = x + (left ? left->sum : 0) + (right ? right->sum : 0);
  }

  auto operator<=>(const S& o) const noexcept { return x <=> o.x; }
  bool operator==(const S& o) const noexcept { return x == o.x; }

  auto operator<=>(int64_t o) const noexcept { return x <=> o; }
  bool operator==(int64_t o) const noexcept { return x == o; }
};

void validate_state(ivl::structs::SplayTree<S>& s) {
  using Side = ivl::structs::SplayTree<S>::Side;
  if (!s)
    return;
  validate_state(s.child(Side::Left));
  validate_state(s.child(Side::Right));
  auto prev_sum = s.value_ptr()->sum;
  s.refresh_state();
  auto next_sum = s.value_ptr()->sum;
  IVL_DBG_ASSERT(prev_sum == next_sum);
}

int main() {
  // ivl::structs::SplayTree<int> s;
  ivl::structs::SplayTree<S> s;

  // s.insert(12);
  // s.insert(42);
  // s.insert(-1);

  // s.insert(2);
  // // s.debug_print_shape();
  // s.insert(3);
  // // s.debug_print_shape();
  // s.insert(1);
  // // s.debug_print_shape();
  // s.insert(0);
  // // s.debug_print_shape();

  for (int i = 0; i < 1000000; ++i)
    s.insert((rand() % 100000));

  LOG(s.max_depth());

  LOG(s.value_ptr()->sum);
}
