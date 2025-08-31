#include "api.hpp"

uint64_t tree_sum(Node* node) {
  return node == nullptr ? 0 : node->data + tree_sum(node->children[0]) + tree_sum(node->children[1]);
}
