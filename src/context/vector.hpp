#include <utility>

#include "context.hpp"
#include "utils.hpp"

namespace ctx {

  template<typename T, typename Alloc>
  struct vector {
    T* begin;
    T* end;
    T* capacity;

    vector() : begin(nullptr), end(nullptr), capacity(nullptr){}

    // copy constructor
    static vector ctx_constructor(ContextWith<Alloc> auto ctx,
                                  const vector& o){
      if (o.empty()) return vector();

      auto& alloc = ctx.get<Alloc>();
      auto mem = alloc.allocate(o.size());

      for (size_t i = 0; i < o.size(); ++i)
        new (mem + i) T (ctx_aware_construct(ctx, o[i]));

      return vector{.begin = mem, .end = mem + o.size(), .capacity = mem + o.size()};
    }

    vector(vector&& o)
      : begin(std::exchange(o.begin, nullptr))
      , end(std::exchange(o.end, nullptr))
      , capacity(std::exchange(o.capacity, nullptr)){}

    void ctx_destructor(ContextWith<Alloc> auto ctx){
      
    }
  };
  
} // namespace ctx
