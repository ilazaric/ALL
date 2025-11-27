#pragma once

#include <ivl/util>
#include <exception>
#include <format>
#include <source_location>
#include <vector>
#include <print>

namespace ivl {

  struct base_exception {
    struct detail_handle {
      base_exception* ptr;
      int             idx;

      detail_handle(base_exception& e) : ptr(&e), idx(std::uncaught_exceptions()) {}
    };

    inline static thread_local std::vector<detail_handle> inflight_exceptions{};

    struct context {
      std::source_location location;
      std::string          text;
    };

    std::string          throw_text;
    std::source_location throw_location;
    std::vector<context> added_context;

    base_exception(
      std::string_view throw_text = "", std::source_location throw_location = std::source_location::current()
    )
        : throw_text(throw_text), throw_location(throw_location) {
      inflight_exceptions.emplace_back(*this);
    }

    ~base_exception() { inflight_exceptions.pop_back(); }

    static bool is_in_flight() {
      return !inflight_exceptions.empty() && inflight_exceptions.back().idx + 1 == std::uncaught_exceptions();
    }

    void dump(std::FILE* stream = stdout) const {
      std::println(
        stream, "ivl::base_exception thrown from {}:'{}':{}", throw_location.file_name(),
        throw_location.function_name(), throw_location.line()
      );
      if (!throw_text.empty()) std::println(stream, " + text: {}", throw_text);
      for (auto&& ctx : added_context) {
        std::println(
          stream, " + added context from {}:'{}':{}", ctx.location.file_name(), ctx.location.function_name(),
          ctx.location.line()
        );
        if (!ctx.text.empty()) std::println(stream, " + + text: {}", ctx.text);
      }
    }
  };

  // TODO: add std::source_location magic
#define EXCEPTION_CONTEXT(...)                                                                                         \
  ::ivl::util::scope_exit _ {                                                                                          \
    [&, EXCEPTION_CONTEXT_exception_count = std::uncaught_exceptions()] {                                              \
      if (std::uncaught_exceptions() == EXCEPTION_CONTEXT_exception_count + 1 &&                                       \
          ::ivl::base_exception::is_in_flight())                                                                       \
        ::ivl::base_exception::inflight_exceptions.back().ptr->added_context.emplace_back(                             \
          std::source_location::current(), std::format(__VA_ARGS__)                                                    \
        );                                                                                                             \
    }                                                                                                                  \
  }

} // namespace ivl
