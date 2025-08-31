#pragma once

#include <cassert>
#include <memory>
#include <utility>
#include <vector>

#include <ivl/alloc/global_alloc_fwd.hpp>

namespace ivl::structs {

  enum class Child { Left = 0, Right = 1 };

  Child sibling(Child C) {
    switch (C) {
    case Child::Left:
      return Child::Right;
    case Child::Right:
      return Child::Left;
    default:
      std::unreachable();
    }
  }

  // ptr + side compressed into same thing
  template <typename Ptr>
  struct PackedPtr {
    static_assert(ivl::extended_pointer_traits<Ptr>::is_faithfully_integral);
    static_assert(ivl::extended_pointer_traits<Ptr>::unused_lower_bits >= 1);
  };

  template <typename V, typename InAlloc = ivl::GlobalAlloc<V>>
  struct BinaryTree {
    struct Node;

    using Alloc = std::allocator_traits<InAlloc>::rebind_alloc<Node>;
    using Ptr   = std::allocator_traits<Alloc>::pointer;

    struct Node {
      V   value;
      Ptr children[2];
      Ptr parent;
      Node(auto&&... args) : value(std::forward<decltype(args)>(args)...), children{}, parent{} {}
    };

    Ptr                         data{};
    [[no_unique_address]] Alloc alloc{};

    BinaryTree()                             = default;
    BinaryTree(const BinaryTree&)            = delete;
    BinaryTree(BinaryTree&& o)               = default;
    BinaryTree& operator=(const BinaryTree&) = delete;
    BinaryTree& operator=(BinaryTree&&)      = default;
    // ~BinaryTree() = default;

  private:
    void destroy_all(Ptr p) {
      if (!p) return;
      destroy_all(p->children[Child::Left]);
      destroy_all(p->children[Child::Right]);
      std::destroy_at(&*p);
      alloc.deallocate(p, 1);
    }

  public:
    ~BinaryTree() { destroy_all(data); }

    void link();

    BinaryTree cut(Child child) {
      IVL_ASSERT(data);
      auto p                = data->children[child];
      data->children[child] = nullptr;
      if (p) p->parent = nullptr;
      return {p, alloc};
    }
  };

} // namespace ivl::structs
