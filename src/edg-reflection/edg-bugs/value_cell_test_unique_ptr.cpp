// ICE - segmentation fault

#include "../value_cell.hpp"

#include <memory>
#include <utility>

using namespace ivl::refl;

constexpr ValueCell<std::unique_ptr<int>> cell;

bool initialize(){
  cell.store(nullptr);
  return true;
}
