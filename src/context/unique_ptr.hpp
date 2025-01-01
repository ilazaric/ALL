#pragma once

#include <utility>
#include <cassert>

#include <ivl/context/context.hpp>

namespace ivl::ctx {

  template<typename T, class Deleter>
  class unique_ptr {
    T* data;

  public:

    constexpr unique_ptr() : data(nullptr){}
    explicit constexpr unique_ptr(T* data) : data(data){}
    constexpr unique_ptr(unique_ptr&& u) : data(std::exchange(u.data, nullptr)){}

    unique_ptr& operator=(unique_ptr&& r){
      std::swap(data, r.data);
      return *this;
    }

    constexpr T* release(){
      return std::exchange(data, nullptr);
    }

    void ctx_destructor(ContextClass<Deleter> ctx){
      ctx.get<Deleter>()(data);
    }

    // destructors don't have arguments, nowhere to put the context :(
    ~unique_ptr(){
      assert(data == nullptr);
    }
  };
  
} // namespace ivl::ctx
