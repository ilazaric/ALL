#include "autodiff"
#include "eval"
#include "point"
#include <iostream>

int ivl_main() {
  // auto evfn = autodiff::comp{autodiff::comp{autodiff::monomial{1, -0.5}, autodiff::norm2{}}, autodiff::delta{}};
  auto evfn = autodiff::comp{autodiff::norm2{}, autodiff::delta{}};
  auto a = random_point();
  auto b = random_point();
  // auto ev1 = evaluate(a, b);
  auto ev1 = dot(a - b, a - b);
  mdarray in(shaped, 6);
  in[0] = a.x;
  in[1] = a.y;
  in[2] = a.z;
  in[3] = b.x;
  in[4] = b.y;
  in[5] = b.z;
  auto ev2 = evfn(in).extract_number();
  std::cout << ev1 << std::endl;
  std::cout << ev2 << std::endl;
  return 0;
}
