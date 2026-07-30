#pragma once

#include <ivl/utility/cpuid>

#include "define2"

#include <format>
#include <utility>

enum reg { EAX, EBX, ECX, EDX };

struct bitrange {
  reg r;
  size_t high;
  size_t low;
  std::string_view identifier;
  bool plus_one = false;
};

consteval std::string describe(reg r) {
  switch (r) {
  case EAX:
    return "eax";
  case EBX:
    return "ebx";
  case ECX:
    return "ecx";
  case EDX:
    return "edx";
  default:
    std::unreachable();
  }
}

consteval std::string describe(bitrange member) {
  return std::format(
    "{{ reg={}, high={}, low={}, id={:?}, plus_one={} }}", describe(member.r), member.high, member.low,
    member.identifier, member.plus_one
  );
}

template<reg r, size_t high, size_t low, bool plus_one>
struct extract_bits {
  size_t operator()(auto&& self) const {
    auto raw = self.raw_cpuid_data;
    auto full = r == EAX ? raw.a : r == EBX ? raw.b : r == ECX ? raw.c : raw.d;
    full >>= low;
    full &= (1ULL << (high - low + 1)) - 1;
    return full + (plus_one ? 1 : 0);
  }
};

consteval std::meta::info
define_cpuid_interpretation(std::meta::info class_type, const std::vector<bitrange>& members) {
  reg current_register = EAX;
  size_t first_unused_bit = 0;
  bool seen_errors = false;
  auto diag_tag = "cpuid_interp";
  std::vector<member_function_description> fns;
  for (size_t i = 0; i < members.size(); ++i) {
    auto member = members[i];
    if (member.identifier.starts_with('_')) {
      __builtin_constexpr_diag(
        34, diag_tag,
        std::format("identifiers cannot start with underscore. member index: {}, member: {}", i, describe(member))
      );
      seen_errors = true;
    }
    if (member.identifier == "RAW_CPUID_DATA") {
      __builtin_constexpr_diag(
        34, diag_tag,
        std::format("identifier 'RAW_CPUID_DATA' is reserved. member index: {}, member: {}", i, describe(member))
      );
      seen_errors = true;
    }
    for (auto c : member.identifier)
      if (c != '_' && (c < 'A' || c > 'Z')) {
        __builtin_constexpr_diag(
          34, diag_tag,
          std::format(
            "identifiers can only contain upper-case letters and underscores. member index: {}, member: {}", i,
            describe(member)
          )
        );
        seen_errors = true;
        break;
      }
    if (!member.identifier.empty())
      for (size_t j = 0; j < i; ++j)
        if (member.identifier == members[j].identifier) {
          __builtin_constexpr_diag(
            34, diag_tag,
            std::format(
              "duplicate identifier. first member index: {}, first member: {}, second member index: {}, second member: "
              "{}",
              j, describe(members[j]), i, describe(member)
            )
          );
          seen_errors = true;
          break;
        }
    if (member.high < member.low) {
      __builtin_constexpr_diag(
        34, diag_tag, std::format("invalid bit-range (low > high). member index: {}, member: {}", i, describe(member))
      );
      __builtin_constexpr_diag(32, diag_tag, "swapping low and high for purposes of future diagnostics");
      seen_errors = true;
      std::swap(member.high, member.low);
    }
    if (member.identifier.empty() && member.plus_one) {
      __builtin_constexpr_diag(
        34, diag_tag,
        std::format("reserved members must not set plus_one. member index: {}, member: {}", i, describe(member))
      );
      __builtin_constexpr_diag(32, diag_tag, "unsetting plus_one for purposes of future diagnostics");
      seen_errors = true;
      member.plus_one = false;
    }
    if (member.high >= 32) {
      __builtin_constexpr_diag(
        34, diag_tag,
        std::format("bit-range outside of [0, 31] bounds. member index: {}, member: {}", i, describe(member))
      );
      __builtin_constexpr_diag(32, diag_tag, "no obvious fix-up, skipping member");
      seen_errors = true;
      continue;
    }
    if (member.r != current_register) {
      if (first_unused_bit != 32) {
        __builtin_constexpr_diag(
          34, diag_tag,
          std::format(
            "jump to different register without fully draining previous. register: {}, first unused bit: {}, member "
            "index: {}, member: {}",
            describe(current_register), first_unused_bit, i, describe(member)
          )
        );
        seen_errors = true;
      }
      if ((int)current_register + 1 != (int)member.r) {
        __builtin_constexpr_diag(
          34, diag_tag,
          std::format(
            "jump to bad register, need to go through them in sequence, EAX,EBX,ECX,EDX. current register: {}, member "
            "index: {}, member: {}",
            describe(current_register), i, describe(member)
          )
        );
        seen_errors = true;
      }
      current_register = member.r;
      first_unused_bit = 0;
    }
    if (first_unused_bit < member.low) {
      __builtin_constexpr_diag(
        34, diag_tag,
        std::format(
          "non-consecutive jump in bit-range. first unused bit: {}, member index: {}, member: {}", first_unused_bit, i,
          describe(member)
        )
      );
      seen_errors = true;
    }
    if (first_unused_bit > member.low) {
      __builtin_constexpr_diag(
        34, diag_tag,
        std::format(
          "overlap in bit-ranges. already used bit: {}, member index: {}, member: {}", first_unused_bit - 1, i,
          describe(member)
        )
      );
      seen_errors = true;
    }
    if (!seen_errors && !member.identifier.empty()) {
      std::string id(member.identifier);
      for (auto& c : id)
        if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
      fns.push_back(member_function_description(
        id, //
        substitute(
          (^^extract_bits), //
          {
            std::meta::reflect_constant(member.r),
            std::meta::reflect_constant(member.high),
            std::meta::reflect_constant(member.low),
            std::meta::reflect_constant(member.plus_one),
          }
        )
      ));
    }
    first_unused_bit = member.high + 1;
  }
  if (seen_errors) {
    __builtin_constexpr_diag(34, diag_tag, "seen errors during validation, bailing out");
    return class_type;
  }
  return define_aggregate_with_member_functions(
    class_type,
    {
      data_member_spec((^^ivl::cpuid_output), {.name = "raw_cpuid_data"}),
    },
    fns
  );
}

struct cpuid_cache_level_description;

consteval {
  define_cpuid_interpretation(
    (^^cpuid_cache_level_description), //
    {
      {EAX, 4, 0, "CACHE_TYPE"},
      {EAX, 7, 5, "CACHE_LEVEL"},
      {EAX, 8, 8, "SELF_INITIALIZING_CACHE"},
      {EAX, 9, 9, "FULLY_ASSOC"},
      {EAX, 13, 10},
      {EAX, 25, 14, "MAX_LP_ADDRESSABLE_IDS", true},
      {EAX, 31, 26, "MAX_CORES_ADDRESSABLE_IDS_PKG", true},

      {EBX, 11, 0, "LINE_SIZE", true},
      {EBX, 21, 12, "PHYS_LINE_PARTITIONS", true},
      {EBX, 31, 22, "NUM_WAYS", true},

      {ECX, 31, 0, "NUM_SETS", true},

      {EDX, 0, 0, "NOT_LWR_CACHE_FLUSH"},
      {EDX, 1, 1, "INCLUSIVE_CACHE"},
      {EDX, 2, 2, "COMPLEX_CACHE_INDEXING"},
      {EDX, 31, 3},
    }
  );
}

cpuid_cache_level_description cpuid_cache_query(uint32_t level) {
  return cpuid_cache_level_description{.raw_cpuid_data = ivl::raw_cpuid(0x4, level)};
}
