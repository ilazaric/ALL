#pragma once

#include <bit>
#include <functional>
#include <iterator>
#include <ivl/util>
#include <optional>
#include <vector>

namespace ivl::structs {

  template <typename T, typename Plus = std::plus<T>>
  struct Turnament {
    [[no_unique_address]] Plus plus;
    std::size_t                length;
    std::vector<T>             data;

    template <typename Rg, typename Pl = Plus>
    Turnament(Rg&& rg, Pl&& plus = Pl {})
        : plus(FWD(plus)), length(std::bit_ceil((std::size_t)std::ranges::distance(rg))),
          data(length * 2) {
      std::size_t idx = 0;
      for (auto&& el : FWD(rg)) {
        data[length + idx++] = FWD(el);
      }
      for (std::size_t idx = length - 1; idx; --idx) {
        data[idx] = plus(data[idx * 2], data[idx * 2 + 1]);
      }
    }

    void set(std::size_t pos, auto&& val) {
      data[pos + length] = FWD(val);
      for (std::size_t idx = (pos + length) / 2; idx; idx /= 2)
        data[idx] = plus(data[idx * 2], data[idx * 2 + 1]);
    }

    const T& leaf(std::size_t pos) const { return data[pos + length]; }

    enum class Side { Left, Right };

    template <Side side = Side::Left>
    std::optional<std::size_t> find(std::size_t lo, std::size_t hi, const auto& pred) {
      // TODO: try some TMP unrolling
      while (true) {
        // TODO: kill opt, return hi
        if (lo == hi)
          return std::nullopt;
        const auto bit_width =
          std::min(std::countr_zero(side == Side::Left ? lo : hi), std::bit_width(hi - lo) - 1);
        auto idx = ((side == Side::Left ? lo : hi - 1) + length) >> bit_width;
        if (pred(data[idx])) {
          // TODO: try to change into bit_width for-loop
          while (idx < length) {
            // TODO: play with this, a lot of variation
            idx = idx * 2 + (side == Side::Right);
            idx ^= !pred(data[idx]);
          }
          return idx - length;
        } else {
          if constexpr (side == Side::Left)
            lo += (1uz << bit_width);
          else
            hi -= (1uz << bit_width);
        }
      }
    }
  };

  template <typename Rg, typename Plus>
  Turnament(Rg&&, Plus&&) -> Turnament<std::ranges::range_value_t<Rg>, std::remove_cvref_t<Plus>>;
  template <typename Rg>
  Turnament(Rg&&) -> Turnament<std::ranges::range_value_t<Rg>>;

} // namespace ivl::structs
