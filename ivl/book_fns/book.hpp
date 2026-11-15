#pragma once

#include <iostream>
#include <map>

namespace ivl::books {

  struct Book {
    std::map</*price*/ double, /*qty*/ double> bids;
    std::map</*price*/ double, /*qty*/ double> asks;

    bool is_degenerate() const { return bids.empty() || asks.empty(); }

    size_t size() const { return bids.size() + asks.size(); }

    Book& operator*=(const double lambda) {
      for (auto& [p, q] : this->bids)
        q *= lambda;
      for (auto& [p, q] : this->asks)
        q *= lambda;
      return *this;
    }

    Book operator*(const double lambda) const {
      Book ret = *this;
      ret *= lambda;
      return ret;
    }

    friend Book operator+(const Book& left, const Book& right) {
      Book ret = left;
      for (auto& [p, q] : right.bids)
        ret.bids[p] += q;
      for (auto& [p, q] : right.asks)
        ret.asks[p] += q;
      return ret;
    }

    double bid_qty() const {
      double qty = 0;
      for (auto [p, q] : bids)
        qty += q;
      return qty;
    }

    double ask_qty() const {
      double qty = 0;
      for (auto [p, q] : asks)
        qty += q;
      return qty;
    }
  };

  std::ostream& operator<<(std::ostream& out, const Book& book) {
    out << std::endl;
    out << "{";
    for (auto [p, q] : book.bids)
      out << "\t" << p;
    out << "\t" << "|";
    for (auto [p, q] : book.asks)
      out << "\t" << p;
    out << "\t" << "}";
    out << std::endl;
    out << "{";
    for (auto [p, q] : book.bids)
      out << "\t" << q;
    out << "\t" << "|";
    for (auto [p, q] : book.asks)
      out << "\t" << q;
    out << "\t" << "}";
    out << std::endl;
    return out;
  }

} // namespace ivl::books
