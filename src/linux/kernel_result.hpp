#pragma once

#include <ivl/util>

namespace ivl::linux {

  /*
    All raw syscalls return a 64bit number, which we intepret as signed.
    An error is iff it is a negative value.
    Only possible errors are in [-4095 == MAX_ERRNO, -1] range.
    (if anything else shows up it's a kernel bug)
   */

  template <size_t width>
    requires(width == 2 || width == 4 || width == 8)
  struct syscall_error {
    std::conditional_t<width == 2, int16_t, std::conditional_t<width == 4, int32_t, int64_t>> value;
  };

  // TODO: this should work with more standard-layout stuff, like long, and {{long}, whatever}
  template <typename T>
    requires std::is_standard_layout_v<T> && std::is_layout_compatible_v<T, syscall_error<sizeof(T)>>
  struct [[nodiscard]] or_syscall_error {
    using error_type = syscall_error<sizeof(T)>;

    union {
      error_type error;
      T          success;
    };

    explicit or_syscall_error(long value) {
      if (value < 0) std::construct_at(&error, value);
      else std::construct_at(&success, value);
    }

    or_syscall_error() = delete;

    bool is_error() const noexcept { return error.value < 0; }
    bool is_success() const noexcept { return error.value >= 0; }

    // unwrap, TODO understand unwinding

    decltype(auto) with(auto&& callable) & { return is_success() ? FWD(callable)(success) : FWD(callable)(error); }
    decltype(auto) with(auto&& callable) const& { return is_success() ? FWD(callable)(success) : FWD(callable)(error); }
    decltype(auto) with(auto&& callable) && {
      return is_success() ? FWD(callable)(std::move(success)) : FWD(callable)(std::move(error));
    }
    decltype(auto) with(auto&& callable) const&& {
      return is_success() ? FWD(callable)(std::move(success)) : FWD(callable)(std::move(error));
    }

    void with_success(auto&& callable) & {
      if (is_error()) return;
      FWD(callable)(success);
    }
    void with_success(auto&& callable) const& {
      if (is_error()) return;
      FWD(callable)(success);
    }
    void with_success(auto&& callable) && {
      if (is_error()) return;
      FWD(callable)(std::move(success));
    }
    void with_success(auto&& callable) const&& {
      if (is_error()) return;
      FWD(callable)(std::move(success));
    }

    ~or_syscall_error() noexcept(noexcept(success.~T())) {
      if (is_success()) {
        success.~T();
      }
    }
  };

  // struct kernel_success {
  //   long value;
  // };

  // struct kernel_error {
  //   long value;
  // };

  // struct unhandled_error_exception {
  //   long value;
  // }

  // struct bad_error_cast_exception {
  //   long value;
  // };

  // // TODO: maybe rename, syscall_return_t ?
  // // TODO: think about nodiscard
  // // This is basically a variant of successes and errors.
  // [[nodiscard]] struct kernel_result {
  //   long value; // TODO: make private

  //   explicit kernel_result(long value) : value(value) {}

  //   bool is_error() const noexcept { return value < 0; }

  //   bool is_success() const noexcept { return value >= 0; }

  //   // TODO: good monadic stuff

  //   kernel_success unwrap() const {
  //     return is_success() ? kernel_success{value} : throw unhandled_error_exception{value};
  //   }

  //   template <typename T>
  //   T unwrap_as() const {
  //     return is_success() ? T{value} : throw unhandled_error_exception{value};
  //   }

  //   kernel_error unwrap_error() const {
  //     return is_error() ? kernel_error{value} : throw bad_error_cast_exception{value};
  //   }

  //   decltype(auto) visit(auto&& callable) const {
  //     return is_success() ? FWD(callable)(kernel_success{value}) : FWD(callable)(kernel_error{value});
  //   }
  // };

} // namespace ivl::linux
