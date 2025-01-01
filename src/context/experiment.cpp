#include "context.hpp"
#include <ivl/util>
#include <ivl/logger>

#include <iostream>

int main(){
  // int x;
  // float y;
  // ivl::ctx::Context<int, ivl::ctx::Mutable<float>, ivl::ctx::Value<long>> ctx(x, y, 12);
  // static_assert(std::is_same_v<decltype(ctx.get<int>()), const int&>);
  // static_assert(std::is_same_v<decltype(ctx.get<float>()), float&>);
  // static_assert(std::is_same_v<decltype(ctx.get<long>()), long>);
  // ivl::ctx::Context<int> ctxi(ctx);
  // ivl::ctx::Context<ivl::ctx::Mutable<float>> ctxf(ctx);
  // ivl::ctx::Context<ivl::ctx::Value<long>> ctxl(ctx);

  {
    struct stdout_t {};
    struct stderr_t {};
    using A = ivl::ctx::Tagged<stdout_t, ivl::ctx::Mutable<std::ostream>>;
    static_assert(std::is_same_v<typename ivl::ctx::detail::Tag<A>::type, stdout_t>);
    // LOG(ivl::util::typestr<typename ivl::ctx::detail::Type<A>::type>());
    static_assert(std::is_same_v<typename ivl::ctx::detail::Type<A>::type, std::ostream&>);
    using Ctx = ivl::ctx::Context<
      ivl::ctx::Tagged<stdout_t, ivl::ctx::Mutable<std::ostream>>,
      ivl::ctx::Tagged<stderr_t, ivl::ctx::Mutable<std::ostream>>
      >;
    // LOG(ivl::util::typestr<decltype(std::cout)>());

    static_assert(std::is_trivially_copyable_v<Ctx>);

    Ctx ctx(std::cout, std::cerr);

    ctx.get<stdout_t>() << "this is stdout!\n";
    ctx.get<stderr_t>() << "this is stderr!\n";
  }
  
}
