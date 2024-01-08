#include "memory_cell.hpp"

using namespace ivl::refl;

consteval bool test1(){
  using Cell = MemoryCell<>;
#define STORE_LOAD_TEST(expr)                   \
  Cell::store(expr);                            \
  if (Cell::load() != expr)                     \
    return false;
  STORE_LOAD_TEST(^int);
  STORE_LOAD_TEST(^char);
  STORE_LOAD_TEST(^short);
  return true;
}

static_assert(test1());
