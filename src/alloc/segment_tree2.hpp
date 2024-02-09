#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <climits>
#include <cstdint>
#include <tuple>
#include <type_traits>

namespace ivl::alloc {

// like SegmentTree, but more compact and template-y
template <std::size_t LeafCount> struct SegmentTree2 {
  static constexpr std::size_t LENGTH = std::bit_ceil(LeafCount);

  static_assert(LENGTH <= (1ULL << 31));

  template <std::size_t N, std::size_t W> struct Packed {
    using Word = std::uint64_t;
    inline static constexpr std::size_t WordWidth = sizeof(Word) * CHAR_BIT;
    // small memory pessimization
    inline static constexpr std::size_t FixedW = std::bit_ceil(W);
    inline static constexpr std::size_t ElemWidth = 2 * FixedW;
    inline static constexpr std::size_t TotalBitCount = N * ElemWidth;
    inline static constexpr std::size_t WordCount =
        (TotalBitCount + WordWidth - 1) / WordWidth;
    inline static constexpr std::size_t ElemCountPerWord =
        WordWidth / ElemWidth;

    std::array<Word, WordCount> words;

    std::pair<std::uint32_t, std::uint32_t> load(std::uint32_t idx) const {
      auto wordidx = idx / ElemCountPerWord;
      // if constexpr (N == 4)
      //   LOG(N, W, FixedW, ElemWidth, ElemCountPerWord, idx, wordidx);
      auto word = words[wordidx];
      word >>= ((idx % ElemCountPerWord) * ElemWidth);
      auto a = word & ((1ULL << FixedW) - 1);
      word >>= FixedW;
      auto b = word & ((1ULL << FixedW) - 1);
      return {a, b};
    }

    void store(std::uint32_t idx, std::uint32_t a, std::uint32_t b) {
      auto shift = (idx % ElemCountPerWord) * ElemWidth;
      auto el = (((Word)b << FixedW) | a) << shift;
      auto elmask = ((1ULL << FixedW << FixedW) - 1) << shift;
      auto wordidx = idx / ElemCountPerWord;
      auto word = words[wordidx];
      auto newword = el | ~elmask & word;
      words[wordidx] = newword;
    }
  };

  /*
    hmm
    number of states
    0 <= left <= max <= 2^K
    1) max <= 2^(K-1)
    --> left is anything between 0 and max
    --> 2^(K-2) * (2^(K-1) + 1) in total
    2) max > 2^(K-1)
    --> left == max
    --> 2^(K-1) in total
   */

  template <std::size_t I> static consteval auto garbage() {
    if constexpr (I + 1 == 0) {
      return std::tuple<>();
    } else {
      return std::tuple_cat(garbage<I - 1>(),
                            std::tuple<Packed<(LENGTH >> I), I + 1>>());
    }
  }

  // template<std::size_t I>
  // struct DataImpl {
  //   using type = std::conditional_t<
  //     I+1==0,
  //     std::tuple<>,
  //     decltype(std::tuple_cat(std::declval<DataImpl<I-1>::type>(),
  //     std::declval<std::tuple<Packed<(LENGTH>>I), I+1>>>()))
  //     >;
  // };

  // using Data = typename DataImpl<std::countr_zero(LENGTH)>::type;

  using Data = decltype(garbage<std::countr_zero(LENGTH)>());

  Data data;

  template <std::size_t Level, bool Kind>
  void modify_single(std::uint32_t idx) {
    if (Kind) {
      std::get<Level>(data).store(idx, 1u << Level, 1u << Level);
    } else {
      std::get<Level>(data).store(idx, 0, 0);
    }
  }

  template <std::size_t Level> void rebuild(std::uint32_t idx) {
    auto [ll, lm] = std::get<Level - 1>(data).load(idx * 2);
    auto [rl, rm] = std::get<Level - 1>(data).load(idx * 2 + 1);
    if (ll == (1 << (Level - 1))) {
      std::get<Level>(data).store(idx, (1u << (Level - 1)) + rl,
                                  (1u << (Level - 1)) + rl);
    } else {
      std::get<Level>(data).store(idx, ll, std::max(lm, rm));
    }
  }

  template <std::size_t I> void init_bigger_levels() {
    if constexpr (I <= std::countr_zero(LENGTH)) {
      for (std::uint32_t i = 0; i < (LENGTH >> I); ++i)
        rebuild<I>(i);
      init_bigger_levels<I + 1>();
    }
  }

  template <std::size_t Level> std::uint64_t sum_level() {
    std::uint64_t res = 0;
    LOG(LENGTH >> Level);
    for (std::uint32_t i = 0; i < (LENGTH >> Level); ++i) {
      res += LOG(i, std::get<Level>(data).load(i)).second;
    }
    return res;
  }

  SegmentTree2() {
    for (std::uint32_t idx = 0; idx < LeafCount; ++idx)
      modify_single<0, true>(idx);
    for (std::uint32_t idx = LeafCount; idx < LENGTH; ++idx)
      modify_single<0, false>(idx);
    init_bigger_levels<1>();
  }

  static constexpr std::uint32_t FAILURE = -1;

  template <std::size_t Level> void refresh_upwards(std::uint32_t idx) {
    if constexpr (Level <= std::countr_zero(LENGTH)) {
      rebuild<Level>(idx);
      refresh_upwards<Level + 1>(idx / 2);
    }
  }

  template <std::size_t Level, bool Kind>
  void modify_prefix(std::uint32_t idx, std::uint32_t leftlen) {
    if constexpr (Level == 0) {
      if (leftlen)
        modify_single<0, Kind>(idx);
      refresh_upwards<1>(idx / 2);
    } else {
      if (leftlen >= (1u << (Level - 1))) {
        modify_single<Level - 1, Kind>(idx * 2);
        leftlen -= (1u << (Level - 1));
        modify_prefix<Level - 1, Kind>(idx * 2 + 1, leftlen);
      } else {
        modify_prefix<Level - 1, Kind>(idx * 2, leftlen);
      }
    }
  }

  template <std::size_t Level>
  std::uint32_t take_impl(std::uint32_t idx, std::uint32_t reqlen) {
    if constexpr (Level + 1 != 0) {
      if (std::get<Level>(data).load(idx).first >= reqlen) {
        modify_prefix<Level, false>(idx, reqlen);
        return idx << Level;
      }
      if constexpr (Level != 0) {
        return take_impl<Level - 1>(
            idx * 2 + (std::get<Level - 1>(data).load(idx * 2).second < reqlen),
            reqlen);
      } else {
        return -1;
      }
    } else {
      // does not happen :D
      // std::unreachable();
      return -1;
    }
  }

  std::uint32_t take(std::uint32_t reqlen) {
    if (std::get<std::countr_zero(LENGTH)>(data).load(0).second < reqlen)
      return FAILURE;
    return take_impl<std::countr_zero(LENGTH)>(0, reqlen);
  }

  template <std::size_t Level>
  void give_impl(std::uint32_t idx, std::uint32_t reqlen, std::uint32_t loc) {
    if constexpr (Level != -1) {
      if (loc == 0) {
        modify_prefix<Level, true>(idx, reqlen);
      }
      if constexpr (Level != 0) {
        if (loc >= (1u << (Level - 1))) {
          loc -= (1u << (Level - 1));
          idx = idx * 2 + 1;
        } else {
          idx = idx * 2;
        }
        give_impl<Level - 1>(idx, reqlen, loc);
      }
    }
  }

  void give(std::uint32_t reqlen, std::uint32_t loc) {
    give_impl<std::countr_zero(LENGTH)>(0, reqlen, loc);
  }
};

} // namespace ivl::alloc
