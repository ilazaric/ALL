#pragma once

#include <ivl/logger>
#include <algorithm>
#include <compare>
#include <span>
#include <utility>
#include <vector>

namespace ivl::algorithm {
template<typename It>
void monotonic_contiguous_join_v1(It a, It b, It c) {
  while (a != b && b != c) {
    std::vector<std::remove_cvref_t<decltype(*a)>> vec;
    size_t vec_end = 0;
    It p = b;
    for (; a != b && (p != c || vec.size() != vec_end); ++a) {
      if (vec.size() == vec_end) {
      p_check:
        if (*a > *p) {
          vec.push_back(std::move(*a));
          *a = std::move(*p++);
        }
        continue;
      }
      if (p == c) {
      v_check:
        if (*a > vec[vec_end]) {
          vec.push_back(std::move(*a));
          *a = std::move(vec[vec_end++]);
        }
        continue;
      }
      if (vec[vec_end] < *p) goto v_check;
      else goto p_check;
    }
    if (a != b) return;
    std::move(vec.begin() + vec_end, vec.end(), b);
    a = b;
    b = p;
  }
}

template<typename It>
void monotonic_contiguous_join_v2(It a, It b, It c) {
  if (a == b || b == c) return;
  std::vector<size_t> perm(c - a);
  {
    size_t i = 0;
    It p = a;
    It q = b;
    while (p != b && q != c) {
      if (*p <= *q) perm[i++] = p++ - a;
      else perm[i++] = q++ - a;
    }
    while (p != b) perm[i++] = p++ - a;
    while (q != c) perm[i++] = q++ - a;
  }
  size_t dead = -1;
  for (size_t i = 0; i < perm.size(); ++i) {
    if (perm[i] <= i || perm[i] == dead) continue;
    for (size_t j = i; perm[j] != i; j = std::exchange(perm[j], dead)) std::iter_swap(a + j, a + perm[j]);
  }
}

#if 0
template<typename It>
struct monotonic_contiguous_join_v3_impl {
  // An <= A(n+1)
  // A and B are unrelated

  struct A {
    size_t n;
  };
  struct B {
    size_t n;
  };

  template<auto Arg, auto Vs>
  inline static constexpr auto Reduced = std::is_same_v<decltype(Arg), decltype(Vs)> && Arg.n > Vs.n
                                         ? decltype(Arg)(Arg.n - 1)
                                         : Arg;

  template<auto... Pieces>
  struct Argument;

  inline static consteval std::meta::info drop_nth_piece(std::meta::info arg, size_t i) {
    auto pieces = template_arguments_of(arg);
    auto dead = pieces[i];
    pieces.erase(pieces.begin() + i);
    for (auto& piece : pieces) piece = constant_of(substitute((^^Reduced), {piece, dead}));
    return substitute(^^Argument, pieces);
  }

  inline static consteval size_t arg_index(std::meta::info arg, auto piece) {
    auto refl = std::meta::reflect_constant(piece);
    size_t ret = 0;
    for (auto candidate : template_arguments_of(arg)) {
      if (candidate == refl) return ret;
      ++ret;
    }
    __builtin_constexpr_diag(34, "", display_string_of(refl));
    throw std::meta::exception("invalid argument", ^^arg_index);
  }

  // template<auto>
  // using ItForPack = It;

  template<auto... Pieces>
  struct Argument {
    // ItForPack<Pieces>... its;
    It its[sizeof...(Pieces) + 1];

    template<A Piece>
    It begin() const {
      return its[arg_index(^^Argument, Piece)];
    }
    template<B Piece>
    It begin() const {
      return its[arg_index(^^Argument, Piece)];
    }
    template<size_t Index>
    It begin() const {
      return its[Index];
    }

    template<A Piece>
    It& begin() {
      return its[arg_index(^^Argument, Piece)];
    }
    template<B Piece>
    It& begin() {
      return its[arg_index(^^Argument, Piece)];
    }
    template<size_t Index>
    It& begin() {
      return its[Index];
    }

    template<A Piece>
    It end() const {
      return its[arg_index(^^Argument, Piece) + 1];
    }
    template<B Piece>
    It end() const {
      return its[arg_index(^^Argument, Piece) + 1];
    }
    template<size_t Index>
    It end() const {
      return its[Index + 1];
    }

    template<A Piece>
    It& end() {
      return its[arg_index(^^Argument, Piece) + 1];
    }
    template<B Piece>
    It& end() {
      return its[arg_index(^^Argument, Piece) + 1];
    }
    template<size_t Index>
    It& end() {
      return its[Index + 1];
    }

    template<size_t Index>
    auto drop() const {
      typename[:drop_nth_piece(^^Argument, Index):] ret;
      for (size_t j = 0; j < Index; ++j) ret.its[j] = its[j];
      for (size_t j = Index; j < sizeof...(Pieces); ++j) ret.its[j] = its[j + 1];
      return ret;
    }
  };

  inline static consteval size_t find_droppable(std::meta::info arg) {
    auto pieces = template_arguments_of(arg);
    for (size_t i = 1; i < pieces.size(); ++i)
      if (constant_of(substitute((^^Reduced), {pieces[i], pieces[i - 1]})) == pieces[i - 1]) return i;
    return 0;
  }

  template<auto... Pieces>
  inline static void compress_solve(Argument<Pieces...> arg) {
    if constexpr (sizeof...(Pieces) == 0) {
      return solve_impl(arg);
    } else if constexpr (std::is_same_v<decltype(Pieces...[0]), B>) {
      Argument<(std::conditional_t<std::is_same_v<decltype(Pieces), A>, B, A>(Pieces.n))...> arg2;
      for (size_t i = 0; i <= sizeof...(Pieces); ++i) arg2.its[i] = arg.its[i];
      return compress_solve(arg2);
    } else if constexpr (constexpr auto index = find_droppable(^^Argument<Pieces...>); index) {
      return compress_solve(arg.template drop<index>());
    } else if constexpr (requires { solve_impl(arg); }) {
      return solve_impl(arg);
    } else {
      consteval {
        std::vector<std::string> vec;
        template for (auto piece : {Pieces...}) {
          vec.push_back(std::format("{}({})", identifier_of(decay(^^decltype(piece))), piece.n));
        }
        __builtin_constexpr_diag(34, "", std::format("need to implement solver for {::}", vec));
      }
    }
  }

  template<auto... Pieces>
  inline static void solve(Argument<Pieces...> arg) {
    template for (constexpr size_t i : std::views::iota(0UZ, sizeof...(Pieces))) {
      if (arg.template begin<i>() == arg.template end<i>()) return solve(arg.template drop<i>());
    }
    return compress_solve(arg);
  }

  template<auto Piece>
  inline static decltype(auto) get(auto&& arg) {
    return *arg.template begin<Piece>();
  }
  template<auto Piece>
  inline static decltype(auto) get_last(auto&& arg) {
    return *(arg.template end<Piece>() - 1);
  }
  template<auto Piece>
  inline static decltype(auto) begin(auto&& arg) {
    return arg.template begin<Piece>();
  }
  template<auto Piece>
  inline static decltype(auto) end(auto&& arg) {
    return arg.template end<Piece>();
  }
  template<auto Piece>
  inline static decltype(auto) size(auto&& arg) {
    return arg.template end<Piece>() - arg.template begin<Piece>();
  }

  inline static void solve_impl(Argument<>) {}
  inline static void solve_impl(Argument<A(0)>) {}

  inline static void solve_impl(Argument<A(0), B(0)> arg) {
    if (get<A(0)>(arg) <= get<B(0)>(arg)) {
      ++begin<A(0)>(arg);
      return solve(arg);
    } else {
      std::swap(get<A(0)>(arg), get<B(0)>(arg));
      return solve(
        Argument<A(1), A(0), B(0)>{
          begin<A(0)>(arg) + 1,
          begin<B(0)>(arg),
          begin<B(0)>(arg) + 1,
          end<B(0)>(arg),
        }
      );
    }
  }

  inline static void solve_impl(Argument<A(1), A(0)> arg) {
    auto [a, b, c] = arg.its;
    std::rotate(a, b, c);
  }

  inline static void solve_impl(Argument<A(1), A(0), B(0)> arg) {
    if (get<A(0)>(arg) >= get<B(0)>(arg)) {
      std::swap(get<A(1)>(arg), get<B(0)>(arg));
      ++begin<A(1)>(arg);
      ++begin<B(0)>(arg);
      return solve(arg);
    } else {
      std::swap(get<A(1)>(arg), get<A(0)>(arg));
      std::swap(get<A(0)>(arg), get<B(0)>(arg));
      return solve(
        Argument<A(1), B(0), A(0), B(1)>{
          begin<A(1)>(arg) + 1,
          begin<A(0)>(arg),
          begin<A(0)>(arg) + 1,
          begin<B(0)>(arg) + 1,
          end<B(0)>(arg),
        }
      );
    }
  }

  // inline static void A1_B_A2(It a1, It b, It a2, It end) {
  //   if (a1 == b) return A_B(b, a2, end);
  //   if (b == a2) return;
  //   if (a2 == end) return A_B(a1, b, a2);

  //   if (*a1 <= *b) return A1_B_A2(a1 + 1, b, a2, end);
  //   else {
  //     std::swap(*a1, *b);
  //     std::swap(*b, *a2);
  //     return A1_B_A2(a1 + 1, b + 1, a2 + 1, end);
  //   }
  // }

  // NOT CORRECT
  // // !!! INTERESTING !!!
  // inline static void solve_impl(Argument<A(0), B(0), A(1)> arg) {
  //   if (get<A(0)>(arg) <= get<B(0)>(arg)) {
  //     ++begin<A(0)>(arg);
  //     return solve(arg);
  //   } else {
  //     std::swap(get<A(0)>(arg), get<B(0)>(arg));
  //     std::swap(get<B(0)>(arg), get<A(1)>(arg));
  //     ++begin<A(0)>(arg);
  //     ++begin<B(0)>(arg);
  //     ++begin<A(1)>(arg);
  //     return solve(arg);
  //   }
  // }

  // A0 B0 A1
  // suppose b0..9 better than a0, and b10 worse than a0
  // a0 a1..a10 A0' b0..b9 b10 B0' A1
  // want: b0..b9 a0 ...
  // swap a0..a10 b0..b10
  // b0..b9 b10 A0' a0 a1..a10 B0' A1
  // swap b10 a0
  // b0..b9 a0 A0' b10 a1..a10 B0' A1
  // [b0..b10> a0 [a1..aN> b10 [aN..aM> b11 ...

  // A0 B0 A1d A2 ; b better

  // A0 B0
  // if b better, its a rotate into correct position
  // how does composition of rotates look?
  // a rotate is (i i+1 i+2 ... j-2 j-1 j) simple cycle

  // 0..A, A..A+B
  // application of rotates for j in A..A+B
  // (k[j] ... j)
  // k strictly increasing

  // (10 ... 100)
  // (11 ... 101)
  // (12 ... 102)
  // (a ... b)
  //   (c ... d)
  // [a,b-1] b [b+1,d]
  // #1:
  // b [a,b-1] [b+1,d]
  // b [a,c-2] [c-1,b-1] [b+1,d-1] d
  // #2:
  // b [a,c-2] d [c-1,b-1] [b+1,d-1]

  // [a,b-1] b [b+1,d-1] d
  // r1: b+1 ... d
  // [a,b-1] b d [b+1,d-1]
  // [a,c-1] [c,b-1] b d [b+1,d-1]
  // [a,c-2] c-1 [c,b-1] b d [b+1,d-1]
  // r2: a ... c-1
  // c-1 [a,c-2] [c,b-1] b d [b+1,d-1]
  // swap c-1 b
  // b [a,c-2] [c,b-1] c-1 d [b+1,d-1]
  // swap c-1 d // small rotate
  // b [a,c-2] [c,b-1] d c-1 [b+1,d-1]
  // r3: [c,b-1] | d c-1
  // b [a,c-2] d c-1 [c,b-1] [b+1,d-1]

  // (a ... b)
  //   (c ... d)
  //     (e ... f)
  // [a,f]
  // [a,b-1] b [b+1,f]
  // r1:
  // b [a,b-1] [b+1,f]
  // b [a,c-2] [c-1,b-1] [b+1,d-1] d [d+1,f]
  // r2:
  // b [a,c-2] d [c-1,b-1] [b+1,d-1] [d+1,f]
  // b [a,c-2] d [c-1,x] [x+1,b-1] [b+1,d-1] [d+1,f-1] f
  // c-a+1 + x-(c-1)+1 == e-a
  // 1 + x+1+1 == e
  // x == e-3
  // b [a,c-2] d [c-1,e-3] [e-2,b-1] [b+1,d-1] [d+1,f-1] f
  // r3:
  // b [a,c-2] d [c-1,e-3] f [e-2,b-1] [b+1,d-1] [d+1,f-1]
  // 1  c-a-1  1   e-c-1   1   b-e+2     d-b-1     f-d-1

  // [a,c-2] [c-1,e-3] [e-2,b-1] b [b+1,d-1] d [d+1,f-1] f
  // [a,c-2] c-1 [c,e-3] [e-2,b-1] b [b+1,d-1] d [d+1,f-1] f
  // rotate a ... c-1
  // c-1 [a,c-2] [c,e-3] [e-2,b-1] b [b+1,d-1] d [d+1,f-1] f
  // swap c-1 b
  // b [a,c-2] ||| [c,e-3] [e-2,b-1] c-1 [b+1,d-1] d [d+1,f-1] f
  // rotate c ... e-1
  // ...

  inline static void solve_impl(Argument<A(0), B(0), A(1)> arg) {
    if (get<A(0)>(arg) <= get<B(0)>(arg)) {
      ++begin<A(0)>(arg);
      return solve(arg);
    } else {
      std::swap(get<A(0)>(arg), get<B(0)>(arg));
      if (size<B(0)>(arg) == 1) {
        std::rotate(begin<A(0)>(arg) + 1, begin<B(0)>(arg), end<B(0)>(arg));
        return;
      }
      return solve(
        Argument<A(1), A(0), B(0), A(2)>{
          begin<A(0)>(arg) + 1,
          begin<B(0)>(arg),
          begin<B(0)>(arg) + 1,
          begin<A(1)>(arg),
          end<A(1)>(arg),
        }
      );
    }
  }

  inline static void solve_impl(Argument<A(1), A(0), A(2)> arg) {
    auto [a, b, c, _] = arg.its;
    std::rotate(a, b, c);
  }

  inline static void solve_impl(Argument<A(1), B(0), A(0)> arg) {
    if (get<A(0)>(arg) <= get<B(0)>(arg)) {
      std::swap(get<A(1)>(arg), get<A(0)>(arg));
      return solve(
        Argument<A(2), B(0), A(1), A(0)>{
          begin<A(1)>(arg) + 1,
          begin<B(0)>(arg),
          begin<A(0)>(arg),
          begin<A(0)>(arg) + 1,
          end<A(0)>(arg),
        }
      );
    } else {
      std::swap(get<A(1)>(arg), get<B(0)>(arg));
      return solve(
        Argument<A(2), A(1), B(0), A(0)>{
          begin<A(1)>(arg) + 1,
          begin<B(0)>(arg),
          begin<B(0)>(arg) + 1,
          begin<A(0)>(arg),
          end<A(0)>(arg),
        }
      );
    }
  }

  inline static void solve_impl(Argument<A(2), A(1), A(0)> arg) {
    auto [a, b, c, d] = arg.its;
    if (b - a <= d - c) {
      std::swap_ranges(a, b, c);
      return solve(
        Argument<A(1), A(2), A(0)>{
          b,
          c,
          c + (b - a),
          d,
        }
      );
    } else {
      std::swap_ranges(b - (d - c), b, c);
      return solve(
        Argument<A(2), A(0), A(1)>{
          a,
          b - (d - c),
          b,
          c,
        }
      );
    }
  }

  inline static void solve_impl(Argument<A(0), B(1), B(0)> arg) {
    if (get<A(0)>(arg) <= get<B(0)>(arg)) {
      ++begin<A(0)>(arg);
      return solve(arg);
    } else {
      std::swap(get<A(0)>(arg), get<B(0)>(arg));
      return solve(
        Argument<A(1), B(1), A(0), B(0)>{
          begin<A(0)>(arg) + 1,
          begin<B(1)>(arg),
          begin<B(0)>(arg),
          begin<B(0)>(arg) + 1,
          end<B(0)>(arg),
        }
      );
    }
  }

  // inline static void A2_B1_A1_B2(It a2, It b1, It a1, It b2, It end) {
  //   if (b1 == a1) return A2_A1_B(a2, b1, b2, end);
  //   if (a2 == b1) return A1_B_A2(b1, a1, b2, end);
  //   if (a1 == b2) return A_B(a2, b1, end); // is it even possible?
  //   if (b2 == end) return A1_B_A2(a2, b1, a1, b2);

  //   if (*a1 <= *b1) {
  //     std::swap(*a2, *a1);
  //     std::swap(*a1, *b2);
  //     return bar(a2 + 1, b1, a1 + 1, b2 + 1, end);
  //   } else {
  //     std::swap(*a2, *b1);
  //     std::swap(*b1, *b2);
  //     return truc(a2 + 1, b1, b1 + 1, a1, b2 + 1, end);
  //   }
  // }

  // inline static void A2_B2_B1_A1_B3(It a2, It b2, It b1, It a1, It b3, It end) {
  //   if (a2 == b2) return A2_A1_B_A3(b2, b1, a1, b3, end);
  //   if (b2 == b1) return A2_B1_A1_B2(a2, b1, a1, b3, end);
  //   if (b1 == a1) return A2_B1_A1_B2(a2, b2, a1, b3, end);
  //   if (a1 == b3) return A_B2_B1_B3(a2, b2, b1, b3, end);
  //   if (b3 == end) return A2_B2_B1_A1(a2, b2, b1, a1, b3);

  //   if (*a1 <= *b1) {
  //     std::swap(*a2, *a1);
  //     return A3_B2_B1_A2_A1_B3(a2 + 1, b2, b1, a1, a1 + 1, b3, end);
  //   } else {
  //     std::swap(*a2, *b1);
  //     std::swap(*b1, *b3);
  //     return A2_B2_B1_A1_B3(a2 + 1, b2, b1 + 1, a1, b3 + 1, end);
  //   }
  // }

  inline static void entry(It a, It b, It c) {
    Argument<A(0), B(0)> arg{a, b, c};
    return solve(arg);
  }
};

template<typename It>
void monotonic_contiguous_join_v3(It a, It b, It c) {
  return monotonic_contiguous_join_v3_impl<It>::entry(a, b, c);
}
#endif

template<typename It>
void monotonic_contiguous_join_v4(It a, It b, It c) {
  if (a == b || b == c) return;
  if (*(b - 1) <= *b) return;
  while (*a <= *b) ++a;
  while (*(c - 1) >= *(b - 1)) --c;

  if (c - b > 3) return monotonic_contiguous_join_v1(a, b, c);

  std::vector<std::pair<size_t, size_t>> rots;

  It low = a;
  // size_t last_x = 0;
  for (It high = b; high != c; ++high) {
    while (*low <= *high) ++low;
    auto x = low - a + (high - b);
    auto y = high - a;
    // LOG(x, y);
    rots.emplace_back(x, y);

    // auto depth = high - b;
    // // if (x >= b - a) depth -= x - (b - a);
    // LOG(last_x, y - depth, y + 1);
    // // std::rotate(a + last_x, a + y - depth, a + y + 1);
    // last_x = x;
  }

  auto rot = [&](size_t lo, size_t hi, size_t n) {
    contract_assert(lo <= hi);
    if (lo == hi) return;
    std::rotate(a + lo, a + hi - n % (hi - lo), a + hi);
  };

  auto swp = [&](size_t lo, size_t hi) { std::iter_swap(a + lo, a + hi); };

  if (rots.size() == 1) {
    auto [x, y] = rots[0];
    rot(x, y + 1, 1);
    // std::rotate(a + x, a + y, a + y + 1);
  }

  if (rots.size() == 2) {
    auto [x, y] = rots[0];
    auto [z, w] = rots[1];
    // rot(x, y + 1, 1);
    // rot(z, w + 1, 1);
    rot(x, z, 1);
    swp(x, y);
    rot(z, y + 1, 2);
    swp(z, w);
  }

  auto dbg = [&] { // LOG(std::format("{}", std::ranges::subrange(a, c)));
  };

  // ( lenA )^N
  // offset ( lenB )^M
  // if offset >= lenA, no decomposition to be done
  // assume N and M are reduced mod lenA and lenB respectively
  // lenX = offset
  // lenY = lenA - offset
  // lenZ = lenB + offset - lenA
  // basically ( lenX )^N ( lenY )^(N+M) ( lenZ )^M, plus some swaps
  // or in other words, how does ( X + Y )^N decompose into ( X )^N ( Y )^N ?

  if (rots.size() == 3) {
    auto [x, y] = rots[0];
    auto [z, w] = rots[1];
    auto [t, u] = rots[2];
    dbg();
    rot(x, z, 1);
    dbg();
    swp(x, y);
    dbg();
    // rot(z, y + 1, 2);
    // rot(t, u + 1, 1);
    if (y < t) {
      if (z == y) swp(y, w);
      else swp(y - 1, w);
      rot(z, y + 1, 2);
      // swp(z, w);
      rot(t, u + 1, 1);
      // swp(z, w + 1);
    } else {
      if (z == y) swp(y, w);
      else swp(y - 1, w);
      // rot(z, y + 1, 2);
      // rot(t, u + 1, 1);
      rot(z, t, 2);
      if (y - z == 1) {
      } else if (t - z == 1) {
        swp(z, y - 1);
        swp(y - 1, y);
      } else if (y - t == 0) {
        swp(z, y);
        swp(z, z + 1);
        // contract_assert(false);
      } else {
        swp(z, y - 1);
        swp(z + 1, y);
      }
      rot(t, y + 1, 2);
      rot(t, y + 1, 1);
      swp(t, u);
      rot(y + 1, u + 1, 1);
      // rot(t, u + 1, 1);
    }
  }
}
} // namespace ivl::algorithm
