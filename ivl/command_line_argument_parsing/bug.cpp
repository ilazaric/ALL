#include <exception>
#include <memory>
#include <source_location>
#include <vector>

namespace ivl {
struct base_exception : std::exception {
  struct detail_handle {
    base_exception* ptr;
    int idx;

    inline detail_handle(base_exception& e) : ptr(&e), idx(std::uncaught_exceptions()) {}
  };

  inline static thread_local std::vector<detail_handle> inflight_exceptions{};

  struct context {
    std::source_location location;
    std::string text;
  };

  inline base_exception(
    std::string_view throw_text = "", std::source_location throw_location = std::source_location::current()
  ) {
    inflight_exceptions.emplace_back(*this);
  }

  inline ~base_exception() { inflight_exceptions.pop_back(); }
};
} // namespace ivl

// IVL disable_ivl_main_handler()

int main() {}
