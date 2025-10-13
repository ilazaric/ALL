#pragma once

#include <ivl/util>

namespace ivl::linux {

  /*
    All raw syscalls return a 64bit number, which we intepret as signed.
    An error is iff it is a negative value.
    Only possible errors are in [-4095 == MAX_ERRNO, -1] range.
    (if anything else shows up it's a kernel bug)
   */

  namespace kernel_result_detail {
    template <typename T, size_t N>
    struct wrapper;

    template <typename T>
    struct wrapper<T, 0> {
      T payload;

      explicit wrapper(auto&&... args) : payload(FWD(args)...) {}

      T&        get() & { return payload; }
      const T&  get() const& { return payload; }
      T&&       get() && { return std::move(payload); }
      const T&& get() const&& { return std::move(payload); }
    };

    template <typename T, size_t N>
      requires(N != 0)
    struct wrapper<T, N> {
      wrapper<T, N - 1> payload;

      explicit wrapper(auto&&... args) : payload(FWD(args)...) {}

      T&        get() & { return payload.get(); }
      const T&  get() const& { return payload.get(); }
      T&&       get() && { return std::move(payload).get(); }
      const T&& get() const&& { return std::move(payload).get(); }
    };

    template <typename A, typename B>
    struct try_align {};

#define _(N, M)                                                                                                        \
  template <typename A, typename B>                                                                                    \
    requires std::is_layout_compatible_v<wrapper<A, N>, wrapper<B, M>>                                                 \
  struct try_align<A, B> {                                                                                             \
    using left_wrapper  = wrapper<A, N>;                                                                               \
    using right_wrapper = wrapper<B, M>;                                                                               \
  }

    _(1, 1);
    _(1, 2);
    _(1, 3);
    _(1, 4);
    _(1, 5);
    _(2, 1);
    _(3, 1);
    _(4, 1);
    _(5, 1);

#undef _

    // template <typename A, typename B>
    // struct try_align {
    //   static_assert(std::is_standard_layout_v<A>);
    //   static_assert(std::is_standard_layout_v<B>);
    //   static_assert(sizeof(A) == sizeof(B));
    //   using
    // };

    // struct try_align {
    //   using left_wrapper;
    //   using right_wrapper;
    // };
  } // namespace kernel_result_detail

  template <size_t width>
    requires(width == 2 || width == 4 || width == 8)
  struct syscall_error {
    std::conditional_t<width == 2, int16_t, std::conditional_t<width == 4, int32_t, int64_t>> value;
  };

  // TODO: this should work with more standard-layout stuff, like long, and {{long}, whatever}
  template <typename T>
    requires std::is_standard_layout_v<T> // && std::is_layout_compatible_v<T, syscall_error<sizeof(T)>>
  struct [[nodiscard]] or_syscall_error {
    using error_type    = syscall_error<sizeof(T)>;
    using left_wrapper  = kernel_result_detail::try_align<T, error_type>::left_wrapper;
    using right_wrapper = kernel_result_detail::try_align<T, error_type>::right_wrapper;
    static_assert(std::is_layout_compatible_v<left_wrapper, right_wrapper>);

    union {
      right_wrapper error;
      left_wrapper  success;
    };

    explicit or_syscall_error(long value) {
      if (value < 0) std::construct_at(&error, value);
      else std::construct_at(&success, value);
    }

    or_syscall_error() = delete;

    bool is_error() const noexcept { return error.get().value < 0; }
    bool is_success() const noexcept { return error.get().value >= 0; }

    // unwrap, TODO understand unwinding

    decltype(auto) with(auto&& callable) & {
      return is_success() ? FWD(callable)(success.get()) : FWD(callable)(error.get());
    }
    decltype(auto) with(auto&& callable) const& {
      return is_success() ? FWD(callable)(success.get()) : FWD(callable)(error.get());
    }
    decltype(auto) with(auto&& callable) && {
      return is_success() ? FWD(callable)(std::move(success.get())) : FWD(callable)(std::move(error.get()));
    }
    decltype(auto) with(auto&& callable) const&& {
      return is_success() ? FWD(callable)(std::move(success.get())) : FWD(callable)(std::move(error.get()));
    }

    void with_success(auto&& callable) & {
      if (is_error()) return;
      FWD(callable)(success.get());
    }
    void with_success(auto&& callable) const& {
      if (is_error()) return;
      FWD(callable)(success.get());
    }
    void with_success(auto&& callable) && {
      if (is_error()) return;
      FWD(callable)(std::move(success.get()));
    }
    void with_success(auto&& callable) const&& {
      if (is_error()) return;
      FWD(callable)(std::move(success.get()));
    }

    ~or_syscall_error() noexcept(noexcept(success.~left_wrapper())) {
      if (is_success()) {
        success.~left_wrapper();
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
