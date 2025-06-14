#include "context.hpp"

using namespace ivl::ctx;

namespace test1 {

  struct A {};
  struct B {};
  struct C {};
  struct D {};
  struct E {};
  struct F {};
  struct G {};
  struct H {};
  struct I {};

  using Ctx =
    Context<A, Mutable<B>, Value<C>, Tagged<D, E>, Tagged<F, Mutable<G>>, Tagged<H, Value<I>>>;

  static_assert(std::is_same_v<decltype(std::declval<Ctx>().get<A>()), const A&>);
  static_assert(std::is_same_v<decltype(std::declval<Ctx>().get<B>()), B&>);
  static_assert(std::is_same_v<decltype(std::declval<Ctx>().get<C>()), C>);
  static_assert(std::is_same_v<decltype(std::declval<Ctx>().get<D>()), const E&>);
  static_assert(std::is_same_v<decltype(std::declval<Ctx>().get<F>()), G&>);
  static_assert(std::is_same_v<decltype(std::declval<Ctx>().get<H>()), I>);
  static_assert(std::is_trivially_copyable_v<Ctx>);

} // namespace test1

namespace test2 {

  struct A {};
  struct B {};
  struct C {};

  void bla(Context<A, B, C>) {
  }
  void bla2(Context<C>) {
  }
  void truc(Context<C, A, B> ctx) {
    bla(ctx);
    bla2(ctx);
  }

} // namespace test2

namespace test3 {

  struct A {};
  struct B {};
  struct C {};
  struct D {};
  struct E {};
  struct F {};
  struct G {};
  struct H {};
  struct I {};

  using Ctx =
    Context<A, Mutable<B>, Value<C>, Tagged<D, E>, Tagged<F, Mutable<G>>, Tagged<H, Value<I>>>;

  void use(Ctx ctx) {
    auto [a, b, c, d, f, h] = ctx.get<A, B, C, D, F, H>();
    static_assert(std::is_same_v<decltype(a), const A&>);
    static_assert(std::is_same_v<decltype(b), B&>);
    static_assert(std::is_same_v<decltype(c), C>);
    static_assert(std::is_same_v<decltype(d), const E&>);
    static_assert(std::is_same_v<decltype(f), G&>);
    static_assert(std::is_same_v<decltype(h), I>);
  }

} // namespace test3
