#pragma once

#include <cstdint>
#include <iostream>

namespace ivl::nt {

  // TODO: add `<typename = decltype([]{})>` if need multiple different types
  struct RTMint {
    inline static int64_t p{1};
    int64_t data;
    RTMint() : data(0){}
    RTMint(int64_t data) : data(data % p){
      if (this->data < 0) this->data += p;
    }
    static RTMint unsafe(int64_t x){
      RTMint ret;
      ret.data = x;
      return ret;
    }
  };

  RTMint operator+(RTMint a, RTMint b){return RTMint::unsafe(a.data + b.data < RTMint::p ? a.data + b.data : a.data + b.data - RTMint::p);}
  RTMint operator-(RTMint a, RTMint b){return RTMint::unsafe(a.data - b.data < 0 ? a.data - b.data + RTMint::p : a.data - b.data);}
  RTMint operator*(RTMint a, RTMint b){return {a.data * b.data};}
  RTMint& operator+=(RTMint& a, RTMint b){return a = a + b;}
  RTMint& operator-=(RTMint& a, RTMint b){return a = a - b;}
  RTMint& operator*=(RTMint& a, RTMint b){return a = a * b;}
  std::ostream& operator<<(std::ostream& out, RTMint r){return out << r.data;}

} // namespace ivl::nt
