#include "monads.hpp"

using namespace ivl::refl;

template <typename T>
T gen();

struct S {
  int bla();
};

void test1() {
  Vec<S> truc;
  truc.fbla();
}

/*

  Vec<Vec<S>> dobije f-memfje od Vec<S>
  Vec<S> naslijedi od std::vector<S>
  pa Vec<S> ima push_back
  ali push_back nije direk

 */

void test2() {
  auto truc = gen<Vec<Vec<S>>>();
  auto znj  = truc.ffbla();
  static_assert(std::is_same_v<decltype(znj), Vec<Vec<int>>>);
}

consteval bool haha() {
  auto i = ^Vec<S>;
  if (bases_of(i).size() != 1)
    return false;
  auto j = type_of(bases_of(i)[0]);
  if (j != ^std::vector<S>)
    return false;
  if (bases_of(j).size() > 5)
    return false;
  return true;
}

static_assert(haha());

consteval bool haha2() {
  auto vec = deep_public_member_functions_of(^std::vector<S>);
  return vec.size() < 55;
}

static_assert(haha2());

void test2o() {
  auto truc = gen<Opt<Opt<S>>>();
  auto znj  = truc.ffbla();
  static_assert(std::is_same_v<decltype(znj), Opt<Opt<int>>>);
}

void test3() {
  auto truc = gen<Vec<Vec<S>>>();
  truc.emplace_back();
  truc.femplace_back();
}

consteval int haha3() {
  int count = 0;
  struct S {
    int&           count;
    constexpr void add() { ++count; }
  };

  Vec<Vec<S>> bla;          // []
  bla.emplace_back();       // [0]
  bla.emplace_back();       // [0 0]
  bla.femplace_back(count); // [1 1]
  bla.emplace_back();       // [1 1 0]
  bla.femplace_back(count); // [2 2 1]
  bla.emplace_back();       // [2 2 1 0]
  bla.femplace_back(count); // [3 3 2 1]
  bla.ffadd();

  // __report_tokens(^{ hello world });

  return count;
}

static_assert(haha3() == 9);

// https://godbolt.org/z/PzY66MWjM
