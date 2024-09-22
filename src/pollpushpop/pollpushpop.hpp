#pragma once

#include <cstdint>
#include <vector>
#include <bit>
#include <type_traits>
#include <tuple>
#include <span>

namespace ivl::pollpushpop {

  using MaskType = std::uint64_t;
  using IndexType = std::uint32_t;
  constexpr MaskType full_mask = ~(MaskType{});
  constexpr MaskType single(IndexType idx){
    return MaskType{1} << idx;
  }

  template<typename T>
  concept MagicIterable = requires(T t, IndexType b){
    {const_cast<const T&>(t).poll()} -> std::same_as<MaskType>;
    {t.push(b)} -> std::same_as<void>;
    {t.pop(b)} -> std::same_as<void>;
  };

  struct Constant {
    const MaskType mask;
    MaskType poll() const {return mask;}
    void push(IndexType){}
    void pop(IndexType){}
  };

  // hmm
  // std::span is capable of both compiletime-known and runtime-known sizes
  // should i take inspiration from it?
  // unsure atm how, TODO
  auto bounded(IndexType count){
    return Constant{(single(0) << count) - 1};
  }

  struct TotalLimit {
    std::uint32_t count;
    MaskType poll() const {return count ? full_mask : 0;}
    void push(IndexType){--count;}
    void pop(IndexType){++count;}
  };

  struct CountLimit {
    IndexType index;
    std::uint32_t count;
    MaskType poll() const {return count ? full_mask : full_mask ^ single(index);}
    void push(IndexType idx){if (idx == index) --count;}
    void pop(IndexType idx){if (idx == index) ++count;}
  };

  
  // TODO: think if this could be faster
  // a lot of unnecessary ifs get run :/
  // [[likely/unlikely]] could help
  struct SymmetryReducer {
    std::vector<std::uint32_t> counts;
    
    MaskType poll() const {
      return single(counts.size() + 1) - 1;
    }
    
    void push(IndexType idx){
      if (idx == counts.size())
        counts.push_back(0);
      ++counts[idx];
    }

    void pop(IndexType idx){
      --counts[idx];
      if (counts.back() == 0)
        counts.pop_back();
    }
  };

  template<MagicIterable ... Ts>
  void iterate(Ts&& ... magics){
    std::vector<MaskType> stack;
    MaskType top;
    MaskType next_top;
    IndexType idx;

    top = (full_mask & ... & magics.poll());
    if (top == 0)
      return;

    goto push_section;

  push_section:
    idx = std::countr_zero(top);
    (magics.push(idx), ...);
    next_top = (full_mask & ... & magics.poll());
    if (next_top == 0){
      goto pop_section;
    } else {
      stack.push_back(top);
      top = next_top;
      goto push_section;
    }

  pop_section:
    idx = std::countr_zero(top);
    (magics.pop(idx), ...);
    top ^= single(idx);
    if (top == 0){
      if (stack.empty()) return;
      top = stack.back();
      stack.pop_back();
      goto pop_section;
    } else {
      goto push_section;
    }

  }

} // namespace ivl::pollpushpop
