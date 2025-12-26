#pragma once

#include <ivl/linux/terminate_syscalls>
#include <ivl/logger>

namespace ivl::linux {

struct mmap_region {
  void* data;
  size_t length;

  mmap_region() : data(nullptr), length(0) {}

  mmap_region(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
      : data(ivl::linux::terminate_syscalls::mmap(addr, length, prot, flags, fd, offset)), length(length) {}

  mmap_region(const mmap_region&) = delete;

  mmap_region(mmap_region&& o) : data(o.data), length(o.length) {
    o.data = nullptr;
    o.length = 0;
  }

  mmap_region& operator=(const mmap_region&) = delete;

  mmap_region& operator=(mmap_region&& o) {
    std::swap(data, o.data);
    std::swap(length, o.length);
    return *this;
  }

  bool empty() const { return data = nullptr; }

  // TODO: clear() -> syscall error
  // ....: to_view() const -> std::string_view
  // ....: to_span() -> std::span<std::byte>
  // ....: to_span() const -> std::span<const std::byte>

  size_t size() const { return length; }

  ~mmap_region() {
    if (data) ivl::linux::terminate_syscalls::munmap(data, length);
  }
};

} // namespace ivl::linux
