#pragma once

#include <cstddef>
#include <vector>

template<size_t N>
struct monomial {
  std::array<size_t, N> orders;
};

template<typename K, size_t N>
struct coef_monomial {
  K coef;
  monomial<N> mono;
};

template<field K, size_t N, typename Order>
struct polynomial {
  std::vector<coef_monomial<N>> monos;

  static polynomial operator+(const polynomial& a, const polynomial& b) {
    auto ap = a.begin();
    auto bp = b.begin();
    Order order;
    polynomial out;
    while (ap != a.end() && bp != b.end()) {
      auto cmp = order(ap->mono, bp->mono);
      if (cmp < 0) {
        out.monos.push_back(*ap);
        ++ap;
        continue;
      }
      if (cmp > 0) {
        out.monos.push_back(*bp);
        ++bp;
        continue;
      }
      if (ap->coef == -bp->coef) {
        ++ap;
        ++bp;
        continue;
      }
      out.monos.emplace_back(ap->coef + bp->coef, ap->mono);
      ++ap;
      ++bp;
    }
    while (ap != a.end()) {
      out.push_back(*ap);
      ++ap;
    }
    while (bp != b.end()) {
      out.push_back(*bp);
      ++bp;
    }
    return out;
  }

  polynomial& operator+=(const polynomial& b) {
    if (monos.empty()) {
      monos = b.monos;
      return *this;
    }

    Order order;
    auto bp = b.begin();
    size_t ap = 0;
    size_t zoff = 0;
    // size_t aqq = monos.size();

    size_t aq = monos.size();
  no_zeroes:
    if (bp == b.end()) {
      std::rotate(monos.begin() + ap, monos.begin() + aq, monos.end());
      return *this;
    }
  no_zeroes_no_b_check:
    if (ap == aq) aq = monos.size();
    auto cmp = order(monos[ap].mono, bp->mono);
    if (cmp < 0) {
      ++ap;
      goto no_zeroes_no_b_check;
    }
    if (cmp > 0) {
      monos.push_back(*bp);
      std::swap(monos[ap], monos.back());
    rev_zoff:
      ++ap;
      ++bp;
      goto no_zeroes;
    }
    if (monos[ap].coef == -bp->coef) goto inc_zoff;
    else monos[ap].coef += bp->coef;
    ++ap;
    ++bp;
    goto no_zeroes;

    if (0) {
    yes_zeroes:
      aq = monos.size();
    }
    while (ap != aq && bp != b.end()) {
      auto cmp = order(monos[ap].mono, bp->mono);
      if (cmp < 0) {
        monos[ap - zoff] = std::move(monos[ap]);
        ++ap;
        continue;
      }
      if (cmp > 0) {
        monos[ap - zoff] = *bp;
        --zoff;
        if (zoff == 0) goto rev_zoff;
        ++bp;
        continue;
      }
      if (monos[ap].coef == -bp->coef)
      inc_zoff:
        ++zoff;
      else {
        monos[ap - zoff] = std::move(monos[ap]);
        monos[ap - zoff].coef += bp->coef;
      }
      ++ap;
      ++bp;
    }
    if (bp == b.end()) {
      std::move(monos.begin() + ap, );
      std::rotate(monos.begin() + ap, monos.begin() + aq, monos.end());
      return *this;
    }
    goto yes_zeroes;
  }
};
