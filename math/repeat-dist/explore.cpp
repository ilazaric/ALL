#include <random>
#include <utility>
#include <ranges>
#include <algorithm>

#include <ivl/logger>
#include <ivl/io/stlutils.hpp>

/*

  r < p --> go next
  r > p --> take r

  f_p : [0,1] -> [0,1]

  f_p(x) =
  * xp : x < p
  * line to 1,1

  f_p o f_q o ...

  want to optimize f^-1(quantile)

  f_p^-1 : (0,0) -- (pp,p) -- (1,1)
  
 */

using Real = double;

Real piecewise(Real x, Real px, Real py){
  if (x < px) return x / px * py;
  else return 1 - (1 - x) / (1 - px) * (1 - py);
}

Real f(Real x, Real p){
  return piecewise(x, p, p*p);
}

Real finv(Real x, Real p){
  return piecewise(x, p*p, p);
}

std::mt19937 gen(13337);
Real rnd(){
  return std::uniform_real_distribution<Real>(0, 1)(gen);
}

int main(){

  if (0){
    Real maxdelta = 0;
    for (int i = 0; i < 10000000; ++i){
      Real x = rnd();
      Real p = rnd();
      Real xx = finv(f(x, p), p);
      Real xxx = f(finv(x, p), p);
      maxdelta = std::max(maxdelta, std::abs(x-xx));
      maxdelta = std::max(maxdelta, std::abs(x-xxx));
    }
    LOG(maxdelta);
  }

  if (0){
    for (Real q = 0.1; q < 0.95; q += 0.1){
    Real pbest = 0;
    Real best = 0;
    for (int i = 0; i < 1000000; ++i){
      Real pcurr = rnd();
      Real curr = finv(q, pcurr);
      if (curr > best) best = curr, pbest = pcurr;
    }
    LOG(q, sqrt(q), pbest, best);
    }
  }

  if (0){
    Real q = 0.2;
    int N = 3;
    LOG(std::pow(q, 1 / std::pow(2, N-1)));

    std::vector<Real> bounds{std::sqrt(q)};
    while (bounds.size()+1 < N) bounds.push_back(std::sqrt(bounds.back()));
    std::ranges::reverse(bounds);
    
    std::vector<Real> samples;
    for (int i = 0; i < 10000000; ++i){
      Real res = rnd();
      for (int j = 0; j+1 < N; ++j){
        if (res > bounds[j]) break;
        res = rnd();
      }
      samples.push_back(res);
    }

    std::ranges::sort(samples);
    LOG(samples[(int)(samples.size()*q)]);
  }

  {
    Real q = 0.2;
    int N = 3;

    std::vector<Real> bounds(N-1, 0.5);
    std::vector<Real> samples(30000);
    auto eval_single = [&]{
      Real res = rnd();
      for (int j = 0; j+1 < N; ++j){
        if (res > bounds[j]) break;
        res = rnd();
      }
      return res;
    };
    auto eval = [&]{
      for (auto& sample : samples)
        sample = eval_single();
      std::ranges::sort(samples);
      return samples[(int)(samples.size() * q)];
    };
    auto optimize = [&](int idx){
      Real prev = bounds[idx];
      Real lo = 0, hi = 1, m1, m2;
      for (int rep = 0; rep < 30; ++rep){
        m1 = (lo*4+hi*3)/7;
        m2 = (lo*3+hi*4)/7;
        bounds[idx] = m1;
        Real e1 = eval();
        bounds[idx] = m2;
        Real e2 = eval();
        if (e1 > e2) hi = m2;
        else lo = m1;
      }
      bounds[idx] = (lo+hi)/2;
      return std::abs(prev - bounds[idx]);
    };

    for (int i = 0; ; ++i){
      if (LOG(i, optimize(i % (N-1))) < 1e-4)
        break;
    }

    LOG(eval(), ivl::io::Elems{bounds});
  }


}
