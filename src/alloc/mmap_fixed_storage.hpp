#pragma once

#include <cstdint>
#include <stdexcept>
#include <cstdio>

#include <sys/mman.h>
#include <string.h>

namespace ivl::alloc {

  // pretty sure this should only be used with static storage duration
  template<std::uintptr_t Location, std::size_t Size = (1ULL << 32)>
  struct MmapFixedStorage {
    MmapFixedStorage(){
      fprintf(stderr, "IVL: MMAPING...\n");
      void* base_ptr = reinterpret_cast<void*>(Location);
      void* mmap_ret = mmap(base_ptr, Size,
                            PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS
                            //| MAP_HUGETLB// | MAP_HUGE_2MB
                            | MAP_FIXED_NOREPLACE
                            ,
                            -1, 0);
      if (mmap_ret == MAP_FAILED){
        auto copy = errno;
        perror("mmap");
        fprintf(stderr, "ERR: %s\n", strerror(copy));
        fprintf(stderr, "ERR: %s\n", strerrorname_np(copy));
        fprintf(stderr, "ERR: maybe set /proc/sys/vm/overcommit_memory to 1 if size > total memory\n");
        throw std::runtime_error("failed to secure storage");
      }
    }

    // this is probably unnecessary, if static storage duration this
    // gets destroyed at end of program, and kernel takes the pages over
    // just in case implementing, maybe other storage duration makes sense?
    // TODO: probably drop throwing and noexcept(false)
    ~MmapFixedStorage() noexcept(false){
      void* base_ptr = reinterpret_cast<void*>(Location);
      int munmap_ret = munmap(base_ptr, Size);
      if (munmap_ret == -1){
        perror("munmap");
        throw std::runtime_error("failed to release storage");
      }
    }
  };

} // namespace ivl::alloc
