#include <ranges>
#include <algorithm>

#include "binary_tree.hpp"

#include <ivl/logger/logger.hpp>
using namespace ivl::logger::default_logger;

template<typename V>
struct IdiotSplay : ivl::trees::SkeletonTree<V> {
  static bool insert(ivl::trees::SkeletonTree<V>& tree, const V& value){
    if (tree.empty()){
      tree = ivl::trees::SkeletonTree<V>::create_node(value);
      return true;
    }

    auto cmp = tree.data->value <=> value;
    if (cmp == 0) return false;
    auto side = cmp > 0 ? ivl::trees::Child::Left : ivl::trees::Child::Right;
    auto ret = insert(tree.child(side), value);
    cmp = tree.child(side).data->value <=> value;
    if (cmp == 0) return ret;
    auto sidebot = cmp > 0 ? ivl::trees::Child::Left : ivl::trees::Child::Right;
    if (side == sidebot){
      tree.rotate(side);
      tree.rotate(side);
    } else {
      tree.child(side).rotate(sidebot);
      tree.rotate(side);
    }
    return ret;
  }
  
  static void splay(ivl::trees::SkeletonTree<V>& tree, const V& value){
    while (true){
      auto cmp1 = tree.data->value <=> value;
      if (cmp1 == 0) return;
      auto side1 = cmp1 > 0 ? ivl::trees::Child::Left : ivl::trees::Child::Right;

      auto cmp2 = tree.child(side1).data->value <=> value;
      if (cmp2 == 0){
        tree.rotate(side1);
        return;
      }
      auto side2 = cmp2 > 0 ? ivl::trees::Child::Left : ivl::trees::Child::Right;

      if (side1 == side2){
        tree.rotate(side1);
        tree.rotate(side1);
      } else {
        tree.child(side1).rotate(side2);
        tree.rotate(side1);
      }
    }
  }

  bool insert(const V& value){
    auto ret = insert(*this, value);
    splay(*this, value);
    return ret;
  }

  static std::size_t depth(const ivl::trees::SkeletonTree<V>& tree){
    if (tree.empty()) return 0;
    return 1 + std::max(depth(tree.child(ivl::trees::Child::Left)),
                        depth(tree.child(ivl::trees::Child::Right)));
  }

  std::size_t depth() const {
    return depth(*this);
  }

  bool test(this IdiotSplay& self){return true;}
};

// template<>
// struct ivl::trees::storage_location<ivl::trees::SkeletonNode<int>> {
//   static constexpr std::uintptr_t value = 0x0000'1030'0000'0000ULL;
// };

static_assert(sizeof(IdiotSplay<int>) == 4);

int main(int argc, char* argv[]){
  assert(argc == 2);
  int len = atoi(argv[1]);
  LOG(len);

  IdiotSplay<int> tree;
  assert(tree.test());
  for (auto i : std::views::iota(0, len))
    tree.insert(i);

  for (auto i : std::views::iota(0, len))
    tree.insert(i);
  for (auto i : std::views::iota(0, len) | std::views::reverse)
    tree.insert(i);
  for (auto i : std::views::iota(0, len))
    tree.insert(0), tree.insert(len-1);

  LOG(tree.depth());
}
