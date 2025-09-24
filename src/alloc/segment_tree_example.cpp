
#include <ivl/logger>

#include "segment_tree.hpp"
#include "segment_tree2.hpp"

#include <ivl/io/stlutils>

int main() {
  LOG(sizeof(ivl::alloc::SegmentTree<(1ULL << 10)>));
  LOG(sizeof(ivl::alloc::SegmentTree2<(1ULL << 10)>));

  LOG(sizeof(ivl::alloc::SegmentTree<(1ULL << 20)>));
  LOG(sizeof(ivl::alloc::SegmentTree2<(1ULL << 20)>));

  ivl::alloc::SegmentTree2<(1ULL << 20)> tree;
  LOG("done constructor");
  LOG(get<20>(tree.data).load(0));

  // LOG(tree.sum_level<9>());
  // LOG(tree.sum_level<8>());
  // LOG(tree.sum_level<7>());
  // LOG(tree.sum_level<6>());
  // LOG(tree.sum_level<5>());
  // LOG(tree.sum_level<4>());
  // LOG(tree.sum_level<3>());
  // LOG(tree.sum_level<2>());
  // LOG(tree.sum_level<1>());
  // LOG(tree.sum_level<0>());

  // LOG(get<9>(tree.data).load(0));
  // LOG(get<9>(tree.data).load(1));

  // LOG(get<8>(tree.data).load(0));
  // LOG(get<8>(tree.data).load(1));
  // LOG(get<8>(tree.data).load(2));
  // LOG(get<8>(tree.data).load(3));
  // LOG(tree.sum_level<8>());

  // LOG(decltype(tree)::LENGTH);

  exit(1);

  // while (true){
  //   LOG(get<12>(tree.data).load(0));
  //   char kind;
  //   std::cin >> kind;
  //   switch (kind){
  //   case 't':
  //     std::uint32_t len;
  //     std::cin >> len;
  //     LOG(len);
  //     LOG(len, tree.take(len));
  //     break;
  //   case 'g':
  //     std::uint32_t loc;
  //     std::cin >> len >> loc;
  //     LOG(len, loc);
  //     tree.give(len, loc);
  //     break;
  //   default:
  //     LOG(kind, "not valid char");
  //     break;
  //   }
  // }

  return 0;
}
