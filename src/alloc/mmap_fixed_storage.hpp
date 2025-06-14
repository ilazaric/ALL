#pragma once

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <stdexcept>

#ifdef __unix__
#include <string.h>
#include <sys/mman.h>
#endif

#ifdef _WIN32
#include <memoryapi.h>
#include <windows.h>
#endif

namespace ivl::alloc {

  // pretty sure this should only be used with static storage duration
  template <std::uintptr_t Location, std::size_t Size = (1ULL << 32)>
  struct MmapFixedStorage {
    MmapFixedStorage() {
      fprintf(stderr, "IVL: MMAPING...\n");
      void* base_ptr = reinterpret_cast<void*>(Location);

#ifdef __unix__
      fprintf(stderr, "IVL: linux\n");
      void* mmap_ret = mmap(base_ptr, Size, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE |
                              MAP_ANONYMOUS
                              //| MAP_HUGETLB// | MAP_HUGE_2MB
                              | MAP_FIXED_NOREPLACE,
                            -1, 0);
      if (mmap_ret == MAP_FAILED) {
        auto copy = errno;
        perror("mmap");
        fprintf(stderr, "ERR: %s\n", strerror(copy));
        fprintf(stderr, "ERR: %s\n", strerrorname_np(copy));
        fprintf(stderr,
                "ERR: maybe set /proc/sys/vm/overcommit_memory to 1 if size > total memory\n");
        throw std::runtime_error("failed to secure storage");
      }
#endif

#ifdef _WIN32
      fprintf(stderr, "IVL: windows\n");
      auto handle =
        CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr,
                           PAGE_READWRITE | SEC_COMMIT, // | SEC_LARGE_PAGES | SEC_COMMIT,
                           (std::uint32_t)(Size >> 32), (std::uint32_t)Size, nullptr);
      assert(handle);
      auto ret = MapViewOfFileEx(handle,
                                 FILE_MAP_WRITE, // | FILE_MAP_LARGE_PAGES,
                                 0, 0, 0, reinterpret_cast<LPVOID>(Location));
      assert(ret);
      assert(reinterpret_cast<std::uintptr_t>(ret) == Location);
#endif
    }

    constexpr char*       data() { return reinterpret_cast<char*>(Location); }
    constexpr std::size_t size() { return Size; }

    // TODO: dropped this for now, think about readding
    // // this is probably unnecessary, if static storage duration this
    // // gets destroyed at end of program, and kernel takes the pages over
    // // just in case implementing, maybe other storage duration makes sense?
    // // TODO: probably drop throwing and noexcept(false)
    // ~MmapFixedStorage() noexcept(false){
    //   void* base_ptr = reinterpret_cast<void*>(Location);
    //   int munmap_ret = munmap(base_ptr, Size);
    //   if (munmap_ret == -1){
    //     perror("munmap");
    //     throw std::runtime_error("failed to release storage");
    //   }
    // }
  };

} // namespace ivl::alloc
