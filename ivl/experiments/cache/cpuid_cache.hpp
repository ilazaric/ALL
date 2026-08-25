#pragma once

// https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
// volume 1, chapter 21: PROCESSOR IDENTIFICATION AND FEATURE DETERMINATION
// CPUID.04H -- Deterministic Cache Parameters

#include <ivl/utility/cpuid>

struct cache_level_description {
  uint32_t cache_type : 5;
  uint32_t cache_level : 3;
  uint32_t self_initializing_cache : 1;
  uint32_t fully_assoc : 1;
  uint32_t reserved_eax_13_10 : 4;
  uint32_t max_lp_addressable_ids : 12;
  uint32_t max_cores_addressable_ids_pkg : 6;
  uint32_t /* eax_end */ : 0;

  uint32_t line_size : 12;
  uint32_t phys_line_partitions : 10;
  uint32_t num_ways : 10;
  uint32_t /* ebx_end */ : 0;

  uint32_t num_sets : 32;
  uint32_t /* ecx_end */ : 0;

  uint32_t not_lwr_cache_flush : 1;
  uint32_t inclusive_cache : 1;
  uint32_t complex_cache_indexing : 1;
  uint32_t reserved_edx_31_3 : 29;
  uint32_t /* edx_end */ : 0;
};

static_assert(sizeof(cache_level_description) == sizeof(ivl::cpuid_output));

#include <meta>
#include <format>

consteval {
  auto type = ^^cache_level_description;
  auto diag_tag = "cache_level_description_validation";
  size_t acc = 0;
  size_t index = 0;
  bool seen_bit_field_error = false;
  for (auto member : members_of(type, std::meta::access_context::unchecked())) {
    // unnamed bit-fields are not considered NSDMs :(
    if (!is_nonstatic_data_member(member) && !is_bit_field(member)) continue;
    if (!is_bit_field(member)) {
      __builtin_constexpr_diag(34, diag_tag, std::format("not a bit-field: {}", display_string_of(member)));
      seen_bit_field_error = true;
    }
    if (!is_public(member)) {
      __builtin_constexpr_diag(34, diag_tag, std::format("not public: {}", display_string_of(member)));
    }
    if (!seen_bit_field_error) {
      auto bits = bit_size_of(member);
      if (bits != 0) {
        acc += bits;
      } else {
        if (acc != 32) {
          __builtin_constexpr_diag(34, diag_tag, std::format("section #{} not of bit size 32: {}", index, acc));
        }
        ++index;
        acc = 0;
      }
    }
  }
  if (!seen_bit_field_error && acc != 0) {
    __builtin_constexpr_diag(34, diag_tag, "missing terminating section delimiter (zero-width bit-field)");
    if (acc != 32) {
      __builtin_constexpr_diag(34, diag_tag, std::format("section #{} not of bit size 32: {}", index, acc));
    }
    ++index;
    acc = 0;
  }
  if (!seen_bit_field_error && index != 4) {
    __builtin_constexpr_diag(34, diag_tag, std::format("wrong number of sections, expected 4, got {}", index));
  }
}
