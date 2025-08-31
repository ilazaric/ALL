#pragma once

#include <algorithm>
#include <bit>
#include <cstdint>

namespace ivl::alloc {

  // this will be used to track large allocations
  // everything used [lo, hi] convention, not [lo, hi>
  // TODO: probably change ^
  // TODO: lo, len might make even more sense
  template <std::size_t LeafCount>
  struct SegmentTree {
    static constexpr std::size_t LENGTH = std::bit_ceil(LeafCount);

    static_assert(LENGTH <= (1ULL << 31));

    // TODO: try manually unrolling stuff

    struct Node {
      std::uint32_t left_free;
      std::uint32_t max_free;

      void rebuild(std::uint32_t len, const Node& left, const Node& right) {
        if (left.left_free == len / 2) {
          left_free = len / 2 + right.left_free;
          max_free  = left_free;
        } else {
          left_free = left.left_free;
          max_free  = std::max(left.max_free, right.max_free);
        }
      }

      template <bool Kind>
      void modify([[maybe_unused]] std::uint32_t len) {
        if constexpr (Kind) {
          left_free = max_free = len;
        } else {
          left_free = max_free = 0;
        }
      }
    };

    Node data[LENGTH * 2];

    SegmentTree() {
      for (std::uint32_t idx = 0; idx < LeafCount; ++idx)
        data[idx + LENGTH].left_free = data[idx + LENGTH].max_free = 1;
      for (std::uint32_t idx = LeafCount; idx < LENGTH; ++idx)
        data[idx + LENGTH].left_free = data[idx + LENGTH].max_free = 1;
      for (std::uint32_t idx = LENGTH - 1; idx; --idx)
        data[idx].rebuild(LENGTH / std::bit_floor(idx), data[idx * 2], data[idx * 2 + 1]);
    }

    static constexpr std::uint32_t FAILURE = -1;

    void refresh_upwards(std::uint32_t idx, std::uint32_t len) {
      while (idx) {
        data[idx].rebuild(len, data[idx * 2], data[idx * 2 + 1]);
        idx /= 2;
        len *= 2;
      }
    }

    template <bool Kind>
    void modify(std::uint32_t idx, std::uint32_t lo, std::uint32_t hi, std::uint32_t leftlen) {
      while (lo != hi) {
        if (leftlen >= (hi - lo + 1) / 2) {
          data[idx * 2].template modify<Kind>((hi - lo + 1) / 2);
          idx = idx * 2 + 1;
          lo  = (lo + hi) / 2 + 1;
          leftlen -= hi - lo + 1;
        } else {
          idx = idx * 2;
          hi  = (lo + hi) / 2;
        }
      }

      if (leftlen) {
        data[idx].template modify<Kind>(1);
      }

      refresh_upwards(idx / 2, 2);
    }

    std::uint32_t take(std::uint32_t reqlen) {
      std::uint32_t idx = 1;
      std::uint32_t lo  = 0;
      std::uint32_t hi  = LENGTH - 1;
      if (data[idx].max_free < reqlen) return FAILURE;

      while (true) {
        if (data[idx].left_free >= reqlen) {
          modify<false>(idx, lo, hi, reqlen);
          return lo;
        }

        if (data[idx * 2].max_free >= reqlen) {
          idx = idx * 2;
          hi  = (lo + hi) / 2;
        } else {
          idx = idx * 2 + 1;
          lo  = (lo + hi) / 2 + 1;
        }
      }

      // std::unreachable();
    }

    void give(std::uint32_t reqlen, std::uint32_t loc) {
      std::uint32_t idx = 1;
      std::uint32_t lo  = 0;
      std::uint32_t hi  = LENGTH - 1;
      while (loc) {
        if (loc >= (hi - lo + 1) / 2) {
          loc -= (hi - lo + 1) / 2;
          idx = idx * 2 + 1;
          lo  = (hi + lo) / 2 + 1;
        } else {
          idx = idx * 2;
          hi  = (hi + lo) / 2;
        }
      }

      modify<true>(idx, lo, hi, reqlen);
    }
  };

} // namespace ivl::alloc
