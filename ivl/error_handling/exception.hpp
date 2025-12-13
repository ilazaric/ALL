#pragma once

namespace ivl::error {

  template <typename T>
  struct context {
    T t;

    ~context() {
      if (std::uncaught_exceptions()) {
        
      }
    }
  };

  struct exception {
    std::string what;
  };

} // namespace ivl::error
