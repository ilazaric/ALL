#pragma once

#include <cassert>
#include <span>
#include <stdexcept>
#include <string_view>

#include <fcntl.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <ivl/fs/filedescriptor.hpp>
#include <ivl/str/nullstringview.hpp>

namespace ivl::fs {

  struct FileView {
    const std::byte* mapped_region;
    std::size_t      offset;
    std::size_t      length;
    // std::size_t dropped_prefix;

    static constexpr std::size_t page_size = 4096;

    FileView(ivl::str::NullStringView path) : FileView(OwnedFD::open(path, O_RDONLY)) {}

    FileView(ivl::str::NullStringView path, std::size_t offset) : FileView(OwnedFD::open(path, O_RDONLY), offset) {}

    FileView(ivl::str::NullStringView path, std::size_t offset, std::size_t length)
        : FileView(OwnedFD::open(path, O_RDONLY), offset, length) {}

    FileView(FD fd) : FileView(fd, 0) {}

    FileView(FD fd, std::size_t offset) : FileView(fd, offset, fstat(fd).st_size) {}

    FileView(FD fd, std::size_t offset, std::size_t length) {
      void* mapped_region = length ? mmap(0, length, PROT_READ, MAP_PRIVATE, fd.get(), offset) : nullptr;
      if (mapped_region == MAP_FAILED) {
        perror("mmap");
        throw std::runtime_error("mmap failed");
      }

      this->mapped_region = static_cast<const std::byte*>(mapped_region);
      this->offset        = offset;
      this->length        = length;
      // this->dropped_prefix = 0;
    }

    FileView(const FileView&) = delete;
    FileView(FileView&& o)
        : mapped_region(o.mapped_region), offset(o.offset), length(o.length)
    // ,dropped_prefix(o.dropped_prefix)
    {
      // o.mapped_region = nullptr;
      o.length = 0;
      // o.dropped_prefix = 0;
    }

    FileView& operator=(const FileView&) = delete;
    FileView& operator=(FileView&& o) {
      if (this != &o) {
        std::swap(mapped_region, o.mapped_region);
        std::swap(offset, o.offset);
        std::swap(length, o.length);
        // std::swap(dropped_prefix, o.dropped_prefix);
      }
      return *this;
    }

    ~FileView() {
      if (length && munmap(const_cast<void*>(static_cast<const void*>(mapped_region)), length)) perror("munmap");
    }

    std::size_t size() const {
      return length; // - dropped_prefix;
    }

    bool empty() const { return size() == 0; }

    std::span<const std::byte> get_remaining() const { return std::span(mapped_region, length); }

    template <typename T>
    std::span<const T> as_span() {
      assert(length % sizeof(T) == 0);
      return std::span<const T>(static_cast<const T*>(static_cast<const void*>(mapped_region)), length / sizeof(T));
    }

    std::string_view as_string_view() {
      return std::string_view(static_cast<const char*>(static_cast<const void*>(mapped_region)), length);
    }

    // void drop_prefix(std::size_t len){
    //   dropped_prefix += len;
    //   if (dropped_prefix > len)
    //     throw std::runtime_error("dropped too much");

    //   if (dropped_prefix >= page_size){

    //   }
    // }
  };

} // namespace ivl::fs
