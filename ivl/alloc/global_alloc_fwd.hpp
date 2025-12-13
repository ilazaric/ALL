#pragma once

namespace ivl::alloc {

  // forward declaration that is a customization point
  // if using stuff that uses this (like ivl/stl/vector)
  // the user needs to define the allocator as well
  template <typename T>
  struct GlobalAlloc;

} // namespace ivl::alloc
