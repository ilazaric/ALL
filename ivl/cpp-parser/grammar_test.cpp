#include <ivl/logger>
#include <ivl/util>
#include "grammar_utils.hpp"

using namespace ivl;
using namespace cppp;
using namespace grammar;

#define STR(type, str)                                                                                                 \
  struct type {                                                                                                        \
    static Result<type> try_parse(std::string_view sv) {                                                               \
      return sv.starts_with(str) ? Result<type>{Consumed{type{}, std::string_view(str).size()}}                        \
                                 : Result<type>{std::unexpected("not " str "?")};                                      \
    }                                                                                                                  \
  }

STR(Hello, "hello");
STR(World, "world");
STR(Space, " ");
STR(One, "1");
STR(Zero, "0");
STR(NewLine, "\n");

ENTITY(Digit, Or<One, Zero>);
ENTITY(Digits, Or<And<Digit, Digits>, Digit>);
ENTITY(Number, Or<Zero, And<One, Digits>>);

template <typename T>
void attempt(std::string_view sv) {
  LOG(util::typestr<T>());
  LOG(sv);
  auto bla = T::try_parse(sv);
  LOG((bool)bla);
  if (bla) LOG(bla.value().consumed);
  else LOG(bla.error());
  std::cerr << std::endl;
}

void test1() {
  attempt<And<Hello, World>>("helloworld");
  attempt<And<Hello, Space, World>>("hello world");
  attempt<And<Hello, World>>("hello world");
}

void test2() {
  attempt<And<Hello, Opt<Space>, World>>("helloworld");
  attempt<And<Hello, Opt<Space>, World>>("hello world");
  attempt<And<Hello, Opt<Space>, World>>("hello  world");
}

void test3() {
  attempt<Or<Hello, World>>("hello");
  attempt<Or<Hello, World>>("world");
}

void test4() {
  attempt<Number>("1010");
  attempt<Number>("0101");
  attempt<Number>("10010110");
}

ENTITY(Number2, Or<Zero, And<One, List<Digit>>>);

void test5() {
  attempt<Number2>("1010");
  attempt<Number2>("0101");
  attempt<Number2>("10010110");
}

int main() {
  test1();
  test2();
  test3();
  test4();
  test5();
}
