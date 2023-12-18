#pragma once

#include <stdexcept>
#include <span>

#include <sys/mman.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <ivl/str/nullstringview.hpp>
#include <ivl/fs/filedescriptor.hpp>

namespace ivl::fs {

  struct FileView {

    const std::byte* mapped_region;
    std::size_t offset;
    std::size_t length;
    std::size_t dropped_prefix;

    static constexpr std::size_t page_size = 4096;

    static std::size_t fstatlen(int fd){
      struct stat stats;
      if (fstat(fd, &stats)){
        perror("fstat");
        throw std::runtime_error("fstat failed");
      }
      return stats.st_size;
    }

    FileView(ivl::str::NullStringView path) :
      FileView(OwnedFD::open(path, O_RDONLY)){}
    
    FileView(ivl::str::NullStringView path,
             std::size_t offset) :
      FileView(OwnedFD::open(path, O_RDONLY), offset){}

    FileView(ivl::str::NullStringView path,
             std::size_t offset,
             std::size_t length) :
      FileView(OwnedFD::open(path, O_RDONLY), offset, length){}

    FileView(FD fd) :
      FileView(fd, 0){}

    FileView(FD fd,
             std::size_t offset) :
      FileView(fd, offset, fstatlen(fd.get())){}

    FileView(FD fd,
             std::size_t offset,
             std::size_t length){

      void* mapped_region = mmap(0, length, PROT_READ, MAP_PRIVATE, fd.get(), offset);
      if (mapped_region == MAP_FAILED){
        perror("mmap");
        throw std::runtime_error("mmap failed");
      }

      this->mapped_region = static_cast<const std::byte*>(mapped_region);
      this->offset = offset;
      this->length = length;
      this->dropped_prefix = 0;
    }

    FileView(const FileView&) = delete;
    FileView(FileView&& o) :
      mapped_region(o.mapped_region),
      offset(o.offset),
      length(o.length),
      dropped_prefix(o.dropped_prefix){
      // o.mapped_region = nullptr;
      o.length = 0;
      o.dropped_prefix = 0;
    }

    FileView& operator=(const FileView&) = delete;
    FileView& operator=(FileView&& o){
      if (this != &o){
        std::swap(mapped_region, o.mapped_region);
        std::swap(offset, o.offset);
        std::swap(length, o.length);
        std::swap(dropped_prefix, o.dropped_prefix);
      }
      return *this;
    }

    ~FileView(){
      if (length)
        if (munmap(const_cast<void*>(static_cast<const void*>(mapped_region)), length))
          perror("munmap");
    }

    std::size_t size() const {
      return length - dropped_prefix;
    }

    bool empty() const {
      return size() == 0;
    }

    std::span<const std::byte> get_remaining() const {
      return std::span(mapped_region, length);
    }

    void drop_prefix(std::size_t len){
      dropped_prefix += len;
      if (dropped_prefix > len)
        throw std::runtime_error("dropped too much");
      
      if (dropped_prefix >= page_size){
        
      }
    }
    
  };

} // namespace ivl::fs
