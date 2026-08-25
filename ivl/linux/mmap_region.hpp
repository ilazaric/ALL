#pragma once

#include "terminate_syscalls"
#include "page_size"
#include <ivl/logger>
#include <cstdint>

namespace ivl::linux {

// struct mmap_region {
//   void* data;
//   size_t length;

// };

struct owned_mmap_region {
  void* data;
  size_t length;

  owned_mmap_region() : data(nullptr), length(0) {}

  owned_mmap_region(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
    : data(reinterpret_cast<void*>(ivl::linux::terminate_syscalls::mmap(reinterpret_cast<long unsigned int>(addr), length, prot, flags, fd, offset))), length(length) {}

  owned_mmap_region(const owned_mmap_region&) = delete;

  owned_mmap_region(owned_mmap_region&& o) : data(o.data), length(o.length) {
    o.data = nullptr;
    o.length = 0;
  }

  owned_mmap_region& operator=(const owned_mmap_region&) = delete;

  owned_mmap_region& operator=(owned_mmap_region&& o) {
    std::swap(data, o.data);
    std::swap(length, o.length);
    return *this;
  }

  bool empty() const { return data == nullptr; }

  // TODO: clear() -> syscall error
  // ....: to_view() const -> std::string_view
  // ....: to_span() -> std::span<std::byte>
  // ....: to_span() const -> std::span<const std::byte>

  size_t size() const { return length; }

  ~owned_mmap_region() {
    if (data) ivl::linux::terminate_syscalls::munmap(reinterpret_cast<long unsigned int>(data), length);
  }
};

template <size_t N>
  requires(N % page_size == 0)
struct alignas(page_size) page_aligned_data {
  char data[N];

  page_aligned_data() = default;

  page_aligned_data(const page_aligned_data&) = delete;
  page_aligned_data(page_aligned_data&&) = delete;

  page_aligned_data& operator=(const page_aligned_data&) = delete;
  page_aligned_data& operator=(page_aligned_data&&) = delete;

  ~page_aligned_data() = default;

  size_t size() const { return N; }

  void* vbegin() { return static_cast<void*>(&data[0]); }
  void* vend() { return static_cast<void*>(&data[0] + N); }

  // mmap_region as_region() { return mmap_region(vbegin(), size()); }

  // TODO: switch to an error type
  [[nodiscard]] int mprotect(int prot) { return raw_syscalls::mprotect(vbegin(), size(), prot); }
};

template <size_t N>
struct poisoned_region {
  page_aligned_data<N> data;
  poisoned_region() { data.mprotect(PROT_NONE); }
  ~poisoned_region() { data.mprotect(PROT_READ | PROT_WRITE); }
};

} // namespace ivl::linux
