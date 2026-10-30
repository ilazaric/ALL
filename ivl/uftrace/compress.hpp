#pragma once

#include <ivl/avl_tree>
#include "default"

namespace ivl::uftrace {
struct compressor_handle {
  size_t index;
  auto operator<=>(const compressor_handle&) const = default;
};

struct compressor_node {
  std::string name;
  std::vector<compressor_handle> calls;
  auto operator<=>(const compressor_node&) const = default;
};

struct compressor {
  std::vector<compressor_node> nodes;
  avl::tree<size_t> tree;

  compressor_node& operator[](compressor_handle h) pre(h.index < nodes.size()) { return nodes[h.index]; }
  const compressor_node& operator[](compressor_handle h) const pre(h.index < nodes.size()) { return nodes[h.index]; }

  compressor_handle insert(compressor_node&& f) {
    std::vector<avl::node<size_t>*> stack{&tree.root};
    while (true) {
      if (stack.back()->empty()) break;
      auto i = (*stack.back())->value;
      auto cmp = nodes[i] <=> f;
      if (cmp == 0) return compressor_handle{i};
      stack.push_back(cmp > 0 ? &(*stack.back())->left : &(*stack.back())->right);
    }
    size_t i = nodes.size();
    nodes.push_back(std::move(f));
    *stack.back() = avl::node<size_t>{std::make_unique<avl::node_struct<size_t>>(i)};
    while (!stack.empty()) {
      update_depth(*stack.back());
      fix_invariant(*stack.back());
      stack.pop_back();
    }
    return compressor_handle{i};
  }

  compressor_handle compress(const node& f) {
    compressor_node c{.name = f.name};
    for (auto&& el : f.calls) c.calls.push_back(compress(el));
    return insert(std::move(c));
  }

  node decompress(compressor_handle h) {
    auto i = h.index;
    contract_assert(i < nodes.size());
    node ret{.name = nodes[i].name};
    for (auto el : nodes[i].calls) ret.calls.push_back(decompress(el));
    return ret;
  }
};
} // namespace ivl::uftrace
