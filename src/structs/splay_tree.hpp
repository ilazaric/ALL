#pragma once

#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>

#include <ivl/debug>
#include <ivl/timer/timer>

namespace ivl::structs {

  template <typename T>
  struct SplayTree {
    enum class Side { Left = 0, Right = 1 };

    static Side sibling(Side s) { return s == Side::Left ? Side::Right : Side::Left; }

    struct Node {
      T          value;
      SplayTree  children[2];
      SplayTree* parent;
      Node(auto&&... args) : value(std::forward<decltype(args)>(args)...), children{}, parent{} {}
    };

    std::unique_ptr<Node> root;

    SplayTree()                 = default;
    SplayTree(const SplayTree&) = delete;
    SplayTree(SplayTree&& o) noexcept : root(std::move(o.root)) { fixup(); }

    SplayTree& operator=(const SplayTree&) = delete;
    SplayTree& operator=(SplayTree&& o) noexcept {
      if (this != &o) {
        root = std::move(o.root);
        fixup();
      }
      return *this;
    }

    ~SplayTree() = default;

    void fixup() noexcept {
      if (empty()) return;
      parent() = nullptr;
      for (auto s : {Side::Left, Side::Right})
        if (child(s)) child(s).parent() = this;
    }

    bool     empty() const noexcept { return !root; }
    explicit operator bool() const noexcept { return (bool)root; }

    SplayTree cut(Side side) {
      IVL_DBG_ASSERT(root);
      auto ret = std::move(child(side));
      refresh_state();
      return ret;
    }

    void link(SplayTree&& new_child, Side side) {
      IVL_DBG_ASSERT(root);
      child(side) = std::move(new_child);
      if (child(side)) child(side).parent() = this;
      refresh_state();
    }

    SplayTree& child(Side side) {
      IVL_DBG_ASSERT(root);
      return root->children[(int)side];
    }

    const SplayTree& child(Side side) const {
      IVL_DBG_ASSERT(root);
      return root->children[(int)side];
    }

    Side child_side(SplayTree& c) const {
      IVL_DBG_ASSERT(root);
      IVL_DBG_ASSERT(&child(Side::Left) == &c || &child(Side::Right) == &c);
      return &child(Side::Left) == &c ? Side::Left : Side::Right;
    }

    T* value_ptr() { return root ? &(root->value) : nullptr; }

    void refresh_state() {
      IVL_DBG_ASSERT(root);
      root->value.refresh_state(child(Side::Left).value_ptr(), child(Side::Right).value_ptr());
    }

    SplayTree*& parent() {
      IVL_DBG_ASSERT(root);
      return root->parent;
    }

    SplayTree* parent() const {
      IVL_DBG_ASSERT(root);
      return root->parent;
    }

    void rotate(Side side) {
      // SCOPE_TIMER;

      IVL_DBG_ASSERT(not empty());
      IVL_DBG_ASSERT(child(side));
      auto p  = parent();
      auto c  = cut(side);
      auto gc = c.cut(sibling(side));
      link(std::move(gc), side);
      // child(side) = std::move(gc);
      c.link(std::move(*this), sibling(side));
      // c.child(sibling(side)) = std::move(*this);
      *this    = std::move(c);
      parent() = p;
    }

    SplayTree* splay(SplayTree* target_parent = nullptr) {
      // SCOPE_TIMER;

      SplayTree* current = this;
      while (current->parent() != target_parent) {
        auto parent = current->parent();
        IVL_DBG_ASSERT(parent);

        auto grandparent = parent->parent();
        if (grandparent == target_parent) {
          parent->rotate(parent->child_side(*current));
          return parent;
        }

        IVL_DBG_ASSERT(grandparent);
        auto ps  = parent->child_side(*current);
        auto gps = grandparent->child_side(*parent);

        if (ps == gps) {
          grandparent->rotate(ps);
          grandparent->rotate(ps);
        } else {
          parent->rotate(ps);
          grandparent->rotate(gps);
        }

        current = grandparent;
      }

      return current;
    }

    void debug_print_shape(int depth = 0) {
      std::cerr << std::string(depth * 2, ' ');
      std::cerr << this << " ";
      if (empty()) std::cerr << 'e' << std::endl;
      else {
        std::cerr << root->value << " " << parent() << std::endl;
        if (!child(Side::Left) && !child(Side::Right)) return;
        child(Side::Left).debug_print_shape(depth + 1);
        child(Side::Right).debug_print_shape(depth + 1);
      }
    }

    std::pair<SplayTree*, bool> insert(auto&& value, SplayTree* target_parent = nullptr) {
      auto parent  = target_parent;
      auto current = this;
      while (not current->empty()) {
        auto cmp = current->root->value <=> value;
        if (cmp == 0) {
          return {current->splay(target_parent), false};
        }
        parent  = current;
        current = &current->child(cmp > 0 ? Side::Left : Side::Right);
      }
      current->root         = std::make_unique<Node>(std::forward<decltype(value)>(value));
      current->root->parent = parent;
      // debug_print_shape();
      return {current->splay(target_parent), true};
    }

    int max_depth() {
      if (empty()) return 0;
      return 1 + std::max(child(Side::Left).max_depth(), child(Side::Right).max_depth());
    }
  };

} // namespace ivl::structs
