#pragma once

#include <memory>
#include <cassert>
#include <utility>
#include <vector>

#include <ivl/alloc/mmap_fixed_storage.hpp>
#include <ivl/alloc/single_element_allocator.hpp>

// should be used only on identifiers
#define FWD(x) (std::forward<decltype(x)>(x))

namespace ivl::trees {

  enum class Child {
    Left = 0,
    Right = 1
  };

  Child sibling(Child C){
    switch (C){
    case Child::Left: return Child::Right;
    case Child::Right: return Child::Left;
    default: std::unreachable();
    }
  }

  template<typename V>
  struct SkeletonTree;

  template<typename V>
  struct SkeletonNode;

  template<typename V>
  struct SkeletonTree {
    // using Alloc = MyAlloc<SkeletonNode<V>>;
    using Alloc = ivl::alloc::SingleElementAllocator<SkeletonNode<V>, 0x0000'1030'0000'0000ULL, (1ULL << 32)>;
    using Ptr = std::unique_ptr<SkeletonNode<V>, typename Alloc::Deleter>;
    
    Ptr data;
    
    // struct Node {
    //   V value;
    //   SkeletonTree children[2];
    //   Node(auto&& ... args) : value(FWD(args)...), children{nullptr, nullptr}{}
    // };

    SkeletonTree() = default;

    SkeletonTree(const SkeletonTree&) = delete;
    SkeletonTree(SkeletonTree&& o) = default;

  private:
    SkeletonTree(Ptr&& o) : data(std::move(o)){}

  public:
    static SkeletonTree create_node(auto&& ... args){
      auto ptr = Alloc::allocate();
      std::construct_at(&*ptr, FWD(args)...);
      return SkeletonTree(Ptr(ptr));
    }

    SkeletonTree& operator=(const SkeletonTree&) = delete;
    SkeletonTree& operator=(SkeletonTree&&) = default;

    ~SkeletonTree() = default;

    SkeletonTree carve(Child C){
      assert(!empty());
      return SkeletonTree(std::move(child(C)));
    }

    void attach(Child C, SkeletonTree&& t){
      assert(!empty());
      assert(child(C).empty());
      child(C) = std::move(t);
    }

    SkeletonTree& child(Child C){
      assert(!empty());
      return data->children[(int)C];
    }

    const SkeletonTree& child(Child C) const {
      assert(!empty());
      return data->children[(int)C];
    }

    // this is a bit faster, no idea why xD
    void rotate(Child C){
      assert(!empty());
      assert(!child(C).empty());
      auto child = carve(C);
      auto grandchild = child.carve(sibling(C));
      attach(C, std::move(grandchild));
      std::swap(child, *this);
      attach(sibling(C), std::move(child));
    }

    // // should be implementable via carve and attach
    // void rotate(Child C){
    //   assert(!empty());
    //   assert(!child(C).empty());
    //   SkeletonTree tmp = std::move(child(C));
    //   child(C) = std::move(tmp.child(sibling(C)));
    //   tmp.child(sibling(C)) = std::move(*this);
    //   *this = std::move(tmp);
    // }

    bool empty() const {return !data;}
    
  };
  
  template<typename V>
  struct SkeletonNode {
    V value;
    SkeletonTree<V> children[2];
    SkeletonNode(auto&& ... args) : value(FWD(args)...), children{}{}
  };

} // namespace ivl::structs::basic_tree
