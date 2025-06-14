#include "book.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <compare>
#include <cstdlib>
#include <iomanip>
#include <ivl/logger>
#include <ivl/util>
#include <random>

using namespace ivl::books;

using Ordering = std::partial_ordering;

std::mt19937 gen(101);

// sum of all subbooks == book
// each subbook has 1 bid 1 ask
// consecutive subbooks share a level
std::vector<Book> carve_up(const Book& book) {
  assert(!book.is_degenerate());

  struct Level {
    double price;
    double qty;
  };
  const auto map2vec = [](const std::map<double, double>& levels_map) {
    std::vector<Level> levels_vec;
    for (auto [p, q] : levels_map)
      levels_vec.emplace_back(p, q);
    return levels_vec;
  };
  std::vector<Level> bids = map2vec(book.bids);
  std::vector<Level> asks = map2vec(book.asks);

  std::ranges::shuffle(bids, gen);
  std::ranges::shuffle(asks, gen);

  std::vector<Book> res;

  while (bids.size() > 1 && asks.size() > 1) {
    res.emplace_back(
      Book {{{bids.back().price, bids.back().qty}}, {{asks.back().price, asks.back().qty}}});
    bids.pop_back();
    asks.pop_back();
  }

  if (bids.size() == 1) {
    for (auto ask : asks) {
      res.emplace_back(Book {{{bids[0].price, bids[0].qty / asks.size()}}, {{ask.price, ask.qty}}});
    }
    return res;
  }

  if (asks.size() == 1) {
    for (auto bid : bids) {
      res.emplace_back(Book {{{bid.price, bid.qty}}, {{asks[0].price, asks[0].qty / bids.size()}}});
    }
    return res;
  }

  // for (size_t i = 1; i < bids.size(); ++i){
  //   res.emplace_back(Book{{{bids[i].price, bids[i].qty}},
  //                         {{asks[0].price, asks[0].qty / (bids.size() - 1)}}});
  // }

  // for (size_t i = 1; i < asks.size(); ++i){
  //   res.emplace_back(Book{{{bids[0].price, bids[0].qty / (asks.size() - 1)}},
  //                         {{asks[i].price, asks[i].qty}}});
  // }

  // while (bids.size() != 1 || asks.size() != 1){
  //   if (bids.size() > asks.size()){
  //     res.emplace_back(Book{{{bids.back().price, bids.back().qty}},
  //                           {{asks.back().price, asks.back().qty / bids.size()}}});
  //     asks.back().qty *= (double)(bids.size()-1) / bids.size();
  //     bids.pop_back();
  //   } else {
  //     res.emplace_back(Book{{{bids.back().price, bids.back().qty / asks.size()}},
  //                           {{asks.back().price, asks.back().qty}}});
  //     bids.back().qty *= (double)(asks.size()-1) / asks.size();
  //     asks.pop_back();
  //   }
  // }

  // res.emplace_back(Book{{{bids.back().price, bids.back().qty}},
  //                       {{asks.back().price, asks.back().qty}}});
  return res;
}

// returns TRUE_ANSWER <=> ans
Ordering try_ans(Book book, const auto& partial_fn, const double ans) {
  while (true) {
    if (book.size() <= 3)
      return partial_fn(book) <=> ans;

    auto subbooks = carve_up(book);
    for (auto& subbook : subbooks)
      subbook *= subbooks.size();
    // for (auto& subbook : subbooks) LOG(subbook, partial_fn(subbook));
    auto   start     = partial_fn(subbooks[0]) <=> ans;
    size_t found_idx = 0;
    for (size_t idx = 1; idx < subbooks.size(); ++idx) {
      auto curr = partial_fn(subbooks[idx]) <=> ans;
      if (start != curr) {
        found_idx = idx;
        break;
      }
    }
    if (found_idx == 0)
      return start;

    // book is sum of subbooks _previously_
    // book is convex combination of subbooks _now_ /* * subbooks.size() */

    const auto left = subbooks[found_idx - 1];
    // const auto left_bid = Book{left.bids, {}};
    // const auto left_ask = Book{{}, left.asks};
    const auto right = subbooks[found_idx];
    // const auto right_bid = Book{right.bids, {}};
    // const auto right_ask = Book{{}, right.asks};
    const auto mid_book = Book {left.bids, right.asks};
    const auto mix      = [&](const double lambda) {
      if (lambda < 1. / 2) {
        const auto a = lambda * 2;
        const auto b = 1 - a;
        return left * b + mid_book * a;
      } else {
        const auto a = lambda * 2 - 1;
        const auto b = 1 - a;
        return mid_book * b + right * a;
      }
      // if (lambda < 1./4){
      //   const auto a = lambda*4;
      //   const auto b = 1-a;
      //   return left + right_bid * a;
      // } else if (lambda < 2./4){
      //   const auto a = lambda*4-1;
      //   const auto b = 1-a;
      //   return left_ask + right_bid + left_bid * b;
      // } else if (lambda < 3./4){
      //   const auto a = lambda*4-2;
      //   const auto b = 1-a;
      //   return left_ask + right_bid + right_ask * a;
      // } else {
      //   const auto a = lambda*4-3;
      //   const auto b = 1-a;
      //   return right + left_ask * b;
      // }
      // return left * (1 - lambda) + right * lambda;
    };

    double lo = 0, hi = 1, mid;
    while (hi - lo > 1e-9) {
      mid = (lo + hi) / 2;
      if ((partial_fn(mix(mid)) <=> ans) == start) {
        lo = mid;
      } else {
        hi = mid;
      }
    }

    auto eqbook = mix((lo + hi) / 2);
    // LOG(ans, partial_fn(eqbook));
    // std::vector<double> lambdas;
    // for (auto [p, q] : eqbook.bids) lambdas.emplace_back(book.bids[p] / q);
    // for (auto [p, q] : eqbook.asks) lambdas.emplace_back(book.asks[p] / q);
    // eqbook *= std::ranges::min(lambdas);

    // book = 1/n * eqbook + (n-1)/n * newbook
    // n/(n-1) book = 1/(n-1) eqbook + newbook

    book *= (double)subbooks.size() / (subbooks.size() - 1);
    eqbook *= 1. / (subbooks.size() - 1);

    for (auto [p, q] : eqbook.bids) {
      book.bids[p] -= q;
      assert(book.bids[p] > -1e5);
      if (book.bids[p] < 1e-5)
        book.bids.erase(p);
    }
    for (auto [p, q] : eqbook.asks) {
      book.asks[p] -= q;
      assert(book.asks[p] > -1e5);
      if (book.asks[p] < 1e-5)
        book.asks.erase(p);
    }
  }
}

// fn is only callable on non-degenerate books with at most 3 levels
double binary_search_ans(const Book& book, const auto& partial_fn) {
  assert(!book.is_degenerate());

  double lo, hi, mid;
  {
    auto subbooks = carve_up(book);
    assert(!subbooks.empty());
    for (auto& subbook : subbooks)
      subbook *= subbooks.size();
    lo = hi = partial_fn(subbooks[0]);
    for (auto&& subbook : subbooks) {
      auto curr = partial_fn(subbook);
      lo        = std::min(lo, curr);
      hi        = std::max(hi, curr);
    }
  }

  const double eps = 1e-9;
  while (hi - lo > eps) {
    mid = (lo + hi) / 2;
    if (try_ans(book, partial_fn, mid) < 0) {
      hi = mid;
    } else {
      lo = mid;
    }
  }

  return (lo + hi) / 2;
}

Book random_book() {
  Book                                   book;
  std::uniform_real_distribution<double> price(1, 10);
  std::uniform_real_distribution<double> qty(1, 100);
  for (int i = 0; i < 10; ++i)
    book.bids[price(gen)] += qty(gen);
  for (int i = 0; i < 10; ++i)
    book.asks[20 + price(gen)] += qty(gen) * 2;
  return book;
}

// void experiment(const Book& book, const auto& total_fn, const std::string_view fn_name){
//   const auto partial_fn = [&](const Book& book){
//     assert(!book.is_degenerate());
//     assert(book.size() <= 3);
//     return total_fn(book);
//   };
//   const auto true_value = total_fn(book);
//   std::vector<double> algo_values;
//   for (int i = 0; i < 10; ++i) algo_values.emplace_back(binary_search_ans(book, partial_fn));
//   std::ranges::sort(algo_values);
//   const auto divergence = std::max(std::abs(true_value - algo_values.front()),
//                                    std::abs(true_value - algo_values.back()));
//   LOG(fn_name);
//   LOG(true_value);
//   for (auto algo_value : algo_values) LOG(algo_value);
//   LOG(divergence);
//   std::cerr << std::endl;
// }

void experiment(const Book& book, const auto& partial_fn, const std::string_view fn_name) {
  const auto protected_partial_fn = [&](const Book& book) {
    assert(!book.is_degenerate());
    assert(book.size() <= 3);
    return partial_fn(book);
  };
  std::vector<double> algo_values;
  for (int i = 0; i < 10; ++i)
    algo_values.emplace_back(binary_search_ans(book, protected_partial_fn));
  std::ranges::sort(algo_values);
  const auto divergence = algo_values.back() - algo_values.front();
  LOG(fn_name);
  // LOG(true_value);
  // for (auto algo_value : algo_values) LOG(algo_value);
  LOG(divergence);
  std::cerr << std::endl;
}

#define EXPERIMENT(book, ...) experiment(book, __VA_ARGS__, #__VA_ARGS__)

void experiment2(const auto& partial_fn, const std::string_view fn_name) {
  Book        left {{{10, 100}}, {{30, 57}}};
  Book        right {{{19, 131}}, {{30, 57}}};
  const auto& mix = [&](double lambda) { return left * (1 - lambda) + right * lambda; };

  LOG(fn_name);
  for (double lambda = 0; lambda < 1; lambda += 0.02)
    LOG(lambda, "\t", partial_fn(mix(lambda)));
  std::cerr << std::endl;
}

#define EXPERIMENT2(...) experiment2(__VA_ARGS__, #__VA_ARGS__)

int main() {
  const auto total_fn = [](const Book& book) {
    assert(!book.is_degenerate());
    double bid_qty = 0;
    for (auto [p, q] : book.bids)
      bid_qty += q;
    double ask_qty = 0;
    for (auto [p, q] : book.asks)
      ask_qty += q;
    // return (bid_qty - ask_qty) / (bid_qty + ask_qty);
    return bid_qty / ask_qty;
  };
  const auto partial_fn = [&](const Book& book) {
    assert(!book.is_degenerate());
    assert(book.size() <= 3);
    return total_fn(book);
  };

  auto book = random_book();

  std::cerr << std::setprecision(12);

  LOG(book);

  // LOG(total_fn(book));
  // LOG(binary_search_ans(book, partial_fn));
  // LOG(binary_search_ans(book, partial_fn));
  // LOG(binary_search_ans(book, partial_fn));
  // LOG(binary_search_ans(book, partial_fn));

  const auto book_pressure = [](const Book& book) {
    return (book.bid_qty() - book.ask_qty()) / (book.bid_qty() + book.ask_qty());
  };

  const auto priced_book_pressure = [](const Book& book) {
    double sum = 0;
    for (auto [p, q] : book.bids)
      sum += p * q;
    for (auto [p, q] : book.asks)
      sum -= p * q;
    return sum / (book.bid_qty() + book.ask_qty());
  };

  EXPERIMENT(book, priced_book_pressure);

  // good
  EXPERIMENT(book, [](const Book& book) { return book.bid_qty() / book.ask_qty(); });
  // good; can be shown to be monotone o ^
  EXPERIMENT(book, book_pressure);
  // bad bc not qty scaling invariant
  // also violates convex in original form
  // satisfies under convex combinations though
  // good in relaxed axioms
  EXPERIMENT(book, [](const Book& book) { return book.bid_qty() - book.ask_qty(); });

  // good
  EXPERIMENT(book, [](const Book& book) {
    double pq = 0;
    for (auto [p, q] : book.bids)
      pq += p * q;
    for (auto [p, q] : book.asks)
      pq -= p * q;
    return pq / (book.bid_qty() + book.ask_qty());
  });

  // good ???
  EXPERIMENT(book, [](const Book& book) {
    double pq = 0;
    for (auto [p, q] : book.bids)
      pq += p * q;
    for (auto [p, q] : book.asks)
      pq -= p * p * q;
    return pq / (book.bid_qty() + book.ask_qty());
  });

  const auto compose = [](const auto& a, const auto& b) {
    return [&](auto&&... args) { return a(b(FWD(args)...)); };
  };

  const auto exp_price_mean = [](auto&& side, const double e) {
    double a = 0, b = 0;
    for (auto [p, q] : side) {
      double c = q * pow(e, p);
      a += c * p;
      b += c;
    }
    return a / b;
  };

  const auto book_ema = [&](Book book) {
    auto bq = book.bid_qty();
    auto aq = book.ask_qty();
    auto ap = exp_price_mean(book.asks, 1 / 1.5);
    auto bp = exp_price_mean(book.bids, 1.5);
    for (auto& [p, q] : book.bids)
      q *= std::pow(1.5, p - bp);
    for (auto& [p, q] : book.asks)
      q *= std::pow(1 / 1.5, p - ap);
    bq /= book.bid_qty();
    aq /= book.ask_qty();
    // for (auto& [p, q] : book.bids) q *= bq;
    // for (auto& [p, q] : book.asks) q *= aq;
    return book;
  };

  // bad
  EXPERIMENT(book, compose(book_pressure, book_ema));

  const auto completer = [](const auto& bigbid, const auto& bigask) {
    return [&](const Book& book) {
      assert(book.size() <= 3);
      assert(book.size() >= 2);
      assert(book.bids.size() >= 1);
      assert(book.asks.size() >= 1);
      if (book.bids.size() == 2)
        return bigbid(book);
      if (book.asks.size() == 2)
        return bigask(book);
      Book bid_copy                                 = book;
      bid_copy.bids[book.bids.begin()->first * 1.5] = 0;
      Book ask_copy                                 = book;
      ask_copy.asks[book.asks.begin()->first / 1.5] = 0;
      auto bv                                       = bigbid(bid_copy);
      auto av                                       = bigask(ask_copy);
      assert(abs(bv - av) < 1e-5);
      return bv;
    };
  };

  EXPERIMENT2(completer(
    [](const Book& book) {
      auto [pb1, qb1] = *book.bids.begin();
      auto [pb2, qb2] = *++book.bids.begin();
      auto [pa, qa]   = *book.asks.begin();
      auto qb         = (pb1 * qb1 * qb1 + pb2 * qb2 * qb2) / (pb1 * qb1 + pb2 * qb2);
      LOG(qb);
      return (qb - qa) / (qb + qa);
    },
    [](const Book& book) {
      auto [pa1, qa1] = *book.asks.begin();
      auto [pa2, qa2] = *++book.asks.begin();
      auto [pb, qb]   = *book.bids.begin();
      auto qa =
        ((1 / pa1) * qa1 * qa1 + (1 / pa2) * qa2 * qa2) / ((1 / pa1) * qa1 + (1 / pa2) * qa2);
      return (qb - qa) / (qb + qa);
    }));

  const auto book2 = random_book();

  EXPERIMENT(book, [&](const Book& book) { return book_pressure(book + book2); });

  return 0;
}
