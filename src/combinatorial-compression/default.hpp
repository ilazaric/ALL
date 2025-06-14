#pragma once

namespace ivl::combcomp {

  template <std::size_t N, typename Head, typename... Tail>
  using SmallIntUtil =
    std::conditional_t<(N - 1 <= std::numeric_limits<Head>::max()), Head, SmallIntUtil<N, Tail...>>;

  // this is smallest unsigned integer
  // that is capable of storing all
  // values strictly less than N
  // since N==0 would be meaningless
  // it instead represents 1 + max size_t
  // (or [0,max size_t] set)
  // universally, represents [0, N-1]
  // it just overflows when N==0
  template <std::size_t N>
  using SmallInt =
    SmallIntUtil<N, std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t, std::size_t>;

  // this represents a value with N states
  // states are represented internally as
  // [0,N-1] integers
  template <std::size_t N>
  struct Value {
    SmallInt<N> data;
  };

  // what if we want a sequence of Values?
  // both std::array and std::tuple are usable
  // std::tuple is more general
  // both have std::get<I> api
  // TODO: what about dynamic stuff like std::vector?
  //       for now i dont care, only comptime stuff

  // std::variant could represent a sum
  // not sure atm i care though

  // TODO: how to test if one sequence has <=> states
  //       than another?
  //       would prefer if i didnt have to
  //       use bignums
  //       maybe just try the stupid conversion
  //       and see if it works?
  //       for now assuming sizes

  template <typename T>
  constexpr std::size_t state_count;

  // this is horrible
  // TODO: try to figure out how to not horrible
  template <typename IT, typename OT>
  concept convertible_to = state_count<IT> <= state_count<OT>;

  template <typename IT, typename OT>
  void convert(const IT& in, OT& out) {
  }

} // namespace ivl::combcomp
