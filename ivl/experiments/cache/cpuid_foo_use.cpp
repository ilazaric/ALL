#include "cpuid_foo"
#include <print>

// IVL add_compiler_flags("-Wno-subobject-linkage")

constexpr auto ctx = std::meta::access_context::unchecked();

int main() {
  for (uint32_t level = 0;; ++level) {
    auto desc = cpuid_cache_query(level);
    if (desc.cache_type() == 0) break;
    template for (constexpr auto member :
                  define_static_array(nonstatic_data_members_of((^^cpuid_cache_level_description), ctx))) {
      if constexpr (identifier_of(member) != "raw_cpuid_data") {
        std::println("{: <30}: {}", identifier_of(member), desc.[:member:]());
      }
    }
    std::println();
  }
}
