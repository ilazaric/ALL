# pointwise
Automatically generate pointwise operations on elements of containers
# example
```
#include <ivl/pointwise.hpp>

int dot_product(std::array<int, 12> a, std::array<int, 12> b){
    using ivl::pointwise::mul;
    int ret = 0;
    for (auto x : a * b)
        ret += x;
    return ret;
}
```

