#pragma once

#include <ivl/linux/raw_syscalls>
#include <ivl/linux/semantic_syscalls>
#include <type_traits>

namespace ivl::linux {
decltype(auto) generic_clone3(auto semantic, const clone_args& clone3_args, auto&& callable) {
  return semantic_syscalls::fat_clone3(
    semantic, &clone3_args, sizeof(clone3_args), const_cast<void*>(reinterpret_cast<const void*>(&callable)),
    +[](void* ptr) noexcept {
      // should not return, or throw
      // (throwing is especially bad as it's allocation)
      static_cast<decltype(callable)>(*reinterpret_cast<std::remove_cvref_t<decltype(callable)>*>(ptr))();
      raw_syscalls::exit_group(1);
    }
  );
}
} // namespace ivl::linux
