#pragma once

#include <iostream>

namespace ivl::io::conversion {

  struct istream_wrapper {
    std::istream* is;

    istream_wrapper(std::istream& is) : is(&is) {}

    template <typename T>
    explicit operator T() {
      T t;
      (*is) >> t;
      return t;
    }
  };

  istream_wrapper cin{std::cin};

} // namespace ivl::io::conversion
