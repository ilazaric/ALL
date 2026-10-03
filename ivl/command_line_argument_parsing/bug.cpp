#include <exception>
#include <memory>
#include <source_location>
#include <vector>

namespace ivl {
struct base_exception : std::exception {
  struct detail_handle {};

  inline static thread_local std::vector<detail_handle> inflight_exceptions{};

  inline base_exception() { inflight_exceptions.emplace_back(); }

  inline ~base_exception() { inflight_exceptions.pop_back(); }
};
} // namespace ivl

// IVL disable_ivl_main_handler()

int main() {}
