#pragma once

#include <ivl/linux/raw_syscalls>
#include <numeric>

namespace ivl::linux {

  struct file_descriptor {
    int value;

    // A file descriptor can't be this value, due to the following snippet from linux source code:
    // unsigned int sysctl_nr_open_max =
    //         __const_min(INT_MAX, ~(size_t)0/sizeof(void *)) & -BITS_PER_LONG;
    // A file descriptor also can't be a negative value, but we have chosen this so errors
    // can be encoded into the negative space (take a look at kernel_result.hpp).
    static constexpr int EMPTY_SENTINEL = std::numeric_limits<int>::max();

    file_descriptor() : value(EMPTY_SENTINEL) {}
    explicit file_descriptor(int value) : value(value) {}

    bool empty() const noexcept { return value == EMPTY_SENTINEL; }
    void clear() noexcept { value = EMPTY_SENTINEL; }

    int get() const noexcept { return value; }
  };

  // Like unique_ptr, closes on destruction.
  struct owned_file_descriptor {
  private:
    int value;

  public:
    owned_file_descriptor() : value(file_descriptor::EMPTY_SENTINEL) {}

    // Make sure you know what you're doing.
    // Takes ownership of the file descriptor.
    explicit owned_file_descriptor(int value) : value(value) {}

    owned_file_descriptor(const owned_file_descriptor&) = delete;
    owned_file_descriptor(owned_file_descriptor&& o) : value(o.value) { o.value = file_descriptor::EMPTY_SENTINEL; }

    owned_file_descriptor& operator=(const owned_file_descriptor&) = delete;
    owned_file_descriptor& operator=(owned_file_descriptor&& o) noexcept {
      if (this == &o) return *this;
      (void)close(); // same TODO as destructor
      value   = o.value;
      o.value = file_descriptor::EMPTY_SENTINEL;
      return *this;
    }

    [[nodiscard]] long close() {
      if (empty()) return 0;
      auto ret = raw_syscalls::close(value);
      value    = file_descriptor::EMPTY_SENTINEL;
      return ret;
    }

    bool empty() const noexcept { return value == file_descriptor::EMPTY_SENTINEL; }

    int get() const noexcept { return value; }

    ~owned_file_descriptor() {
      // TODO: this discards errors silently, this is bad (though for close specifically not super bad).
      // Think what is possible here.
      (void)close();
    }

    explicit(false) operator file_descriptor() const { return file_descriptor{value}; }
  };

} // namespace ivl::linux
