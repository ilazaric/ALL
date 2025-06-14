#pragma once

#include <cstdio>

// example: auto fd = CHECKED(open)("path", O_RDONLY);
#define CHECKED(fn)                                                                                \
  [&](auto&&... args) {                                                                            \
    auto ret = fn(args...);                                                                        \
    if (ret == -1) {                                                                               \
      perror(#fn);                                                                                 \
      exit(-1);                                                                                    \
    }                                                                                              \
    return ret;                                                                                    \
  }
