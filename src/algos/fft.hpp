#pragma once

#include <ivl/timer/timer.hpp>

namespace ivl::algos {

  // `e` should be `len`-th root of unity
  // `len` should be a power of 2
  template<typename T>
  void fft(const T* in, T* out, std::size_t len, T e){

    SCOPE_TIMER;

    {
      out[0] = in[0];
      auto inidx = 0_u32, outidx = 0_u32;
      for (auto idx : std::views::iota(1_u32, len)){
        auto bd = std::countr_zero(idx);
        inidx ^= (1 << bd);
        outidx ^= ((len/2) >> bd);
        out[outidx] = in[inidx];
      }
    }

    // std::vector<T> es(std::countr_zero(len));
    // es[0] = e;
    // for (auto idx : std::views::iota(1_u32, es.size()))
    //   es[idx] = es[idx-1] * es[idx-1];
    // std::ranges::reverse(es);

    if (constexpr auto curr_len = 2_u32; curr_len <= len){
      for (auto off : std::views::iota(0_u32, len / curr_len)){
        T x = out[off * curr_len];
        T y = out[off * curr_len + curr_len/2];
        out[off * curr_len] = x + y;
        out[off * curr_len + curr_len/2] = x - y;
      }
    }

    std::vector<T> es(len);
    es[0] = T{1};
    for (auto idx : std::views::iota(1_u32, len))
      es[idx] = e * es[idx-1];
    
    for (auto curr_len = 4_u32; curr_len <= len; curr_len *= 2){
      // auto ce = es[std::countr_zero(curr_len)-1];
      // ^ e^(len/curr_len*2)
      auto delta = len / curr_len;
      for (auto off : std::views::iota(0_u32, len / curr_len)){
        // T pe = 1;
        for (auto idx : std::views::iota(0_u32, curr_len/2)){
          T x = out[off * curr_len + idx];
          T y = out[off * curr_len + idx + curr_len/2] * es[delta * idx];//pe;
          out[off * curr_len + idx] = x + y;
          out[off * curr_len + idx + curr_len/2] = x - y;
          // pe *= ce;
        }
      }
    }
  }


} // namespace ivl::algos
