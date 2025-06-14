// parent_of(^local class) gives garbage

#include <experimental/meta>
#include <string>
#include <string_view>

consteval void describe(std::meta::info i) {
  std::__report_constexpr_value("satisfies:\n");
#define X(f)                                                                                       \
  if (f(i))                                                                                        \
    std::__report_constexpr_value(#f "\n");
  // X(is_public);
  // X(is_protected);
  // X(is_private);
  // X(is_accessible);
  X(is_virtual);
  X(is_deleted);
  X(is_defaulted);
  X(is_explicit);
  X(is_override);
  X(is_pure_virtual);
  X(is_bit_field);
  X(has_static_storage_duration);
  X(is_nsdm);
  X(is_base);
  X(is_namespace);
  X(is_function);
  // X(is_static);
  X(is_variable);
  X(is_type);
  X(is_alias);
  X(is_incomplete_type);
  X(is_template);
  X(is_function_template);
  X(is_variable_template);
  X(is_class_template);
  X(is_alias_template);
  X(has_template_arguments);
  X(is_constructor);
  X(is_destructor);
  X(is_special_member);
#undef X
  std::__report_constexpr_value("description done\n\n");
}

consteval void print(std::meta::info i) {
  std::string str {name_of(i)};
  std::__report_constexpr_value(str.data());
  std::__report_constexpr_value("\n");
}

consteval bool fn() {
  struct L {};
  auto i  = ^L;
  auto pi = parent_of(i); // this is some garbage
  print(pi);
  describe(pi);
  return false;
}

static_assert(fn());
