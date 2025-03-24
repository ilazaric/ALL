#pragma once

#include <cstdint>

struct Node {
  uint64_t data;
  Node* children[2]{};
};

// users implement this
uint64_t tree_sum(Node* root);
