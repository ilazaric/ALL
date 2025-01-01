#pragma once

#include <type_traits>
#include <memory>

#include "context.hpp"

#define FWD(...) std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

namespace ivl::ctx {

  template<typename T>
  T ctx_aware_construct(Context auto ctx,
                        auto&&... args){
    using T = std::remove_pointer_t<decltype(addr)>;
    if constexpr (requires {T::ctx_constructor(ctx, FWD(args)...) -> T;}){
      return T::ctx_constructor(ctx, FWD(args)...);
    } else {
      return T(FWD(args)...);
    }
  }

  // auto bind_ctx(auto& ctx, auto& obj){
    
  // }
  
} // namespace ivl::ctx
