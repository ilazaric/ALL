#pragma once

namespace ivl::lambda2 {

  template <std::size_t N>
  struct Arg {};

  namespace placeholders {
    inline constexpr Arg<1>  _1;
    inline constexpr Arg<2>  _2;
    inline constexpr Arg<3>  _3;
    inline constexpr Arg<4>  _4;
    inline constexpr Arg<5>  _5;
    inline constexpr Arg<6>  _6;
    inline constexpr Arg<7>  _7;
    inline constexpr Arg<8>  _8;
    inline constexpr Arg<9>  _9;
    inline constexpr Arg<10> _10;
  } // namespace placeholders

  template <typename T>
  struct Holder;

} // namespace ivl::lambda2
