#pragma once

#include <cstddef>
#include <memory>

namespace ivl::avl {
enum class child { left, right };

template<typename T>
struct node_struct;

template<typename T>
struct node {
  std::unique_ptr<node_struct<T>> value;

  size_t depth() const { return value ? value->depth : 0; }

  bool empty() const { return !value; }
  operator bool() const { return (bool)value; }

  // bool leaf() const { return value && !value->left && !value->right; }

  node_struct<T>& operator*() { return *value; }
  const node_struct<T>& operator*() const { return *value; }

  node_struct<T>* operator->() { return value.get(); }
  const node_struct<T>* operator->() const { return value.get(); }

  node& operator()(child c) { return c == child::left ? value->left : value->right; }
  const node& operator()(child c) const { return c == child::left ? value->left : value->right; }
};

template<typename T>
struct node_struct {
  T value;
  size_t depth = 1;
  node<T> left{};
  node<T> right{};
};

template<typename T>
void update_depth(node<T>& root) {
  contract_assert(root);
  root->depth = 1 + std::max(root->left.depth(), root->right.depth());
}

//     y
//    / \
//   x   c
//  / \
// a   b
// -------
//   x
//  / \
// a   y
//    / \
//   b   c

template<typename T>
void rotate(node<T>& root, child A, child B) {
  contract_assert(root);
  contract_assert(root(A));
  auto p = std::move(root);
  root = std::move(p(A));
  p(A) = std::move(root(B));
  root(B) = std::move(p);
  update_depth(root(B));
  update_depth(root);
}

template<typename T>
bool check_invariant(const node<T>& root, size_t depth = 0) {
  if (!root) return true;
  auto l = root->left.depth();
  auto r = root->right.depth();
  if (l > r + 1 || r > l + 1) return false;
  if (depth == 0) return true;
  return check_invariant(root->left, depth - 1) && check_invariant(root->right, depth - 1);
}

//     z!
//    / \
//   x2  d0
//  / \
// a0  y1
//    / \
//   b   c
// -----------
//       z
//      / \
//     y   d
//    / \
//   x   c
//  / \
// a   b
// -----------
//      y
//     / \
//    /   \
//   x     z
//  / \   / \
// a   b c   d

template<typename T>
void fix_invariant(node<T>& root) {
  if (check_invariant(root)) return;
  auto l = root->left.depth();
  auto r = root->right.depth();
  auto A = l > r ? child::left : child::right;
  auto B = l > r ? child::right : child::left;
  if (root(A)(A).depth() < root(A)(B).depth()) rotate(root(A), B, A);
  rotate(root, A, B);
  contract_assert(check_invariant(root, 2));
}

template<typename T>
void insert(node<T>& root, const T& value) {
  if (!root) {
    root.value = std::make_unique<node_struct<T>>(value);
    return;
  }
  auto cmp = root->value <=> value;
  if (cmp == 0) return;
  insert(cmp > 0 ? root->left : root->right, value);
  fix_invariant(root);
}

template<typename T>
struct tree {
  node<T> root;
  void insert(const T& value) { ::ivl::avl::insert(root, value); }
};
} // namespace ivl::avl
