#include <ivl/reflection/argument_parsing>
#include <ivl/reflection/prettier_types>
#include <ivl/reflection/utility>
#include <ivl/utility>
#include <format>
#include <print>

struct cc_bundle {
  bool bla;
  std::string truc;
};

struct args_bundle : cc_bundle {
  cc_bundle c1;
  bool foo;
  cc_bundle c2;
};

/*
A
: B {}
: C
  : D {}
  {}
{}
 */

// template <std::formattable<char> T>
// void describe(const T& arg, size_t indent = 0) {

//   std::println("{}: {}", ivl::reflection::display_string_of(^^T), arg);
// }

consteval void showit(std::meta::info i) { __builtin_constexpr_diag(2, "", display_string_of(i)); }

template <typename T>
concept debug_formattable =
  std::formattable<T, char> && requires(std::formatter<std::remove_cvref_t<T>> f) { f.set_debug_format(); };

template <typename T>
void describe(const T& arg, size_t indent = 0) {
  std::println("{}", ivl::reflection::display_string_of(^^T));
  if constexpr (std::formattable<T, char>) {
    if constexpr (debug_formattable<T>) std::println("{: <{}}= {:?}", "", indent, arg);
    else std::println("{: <{}}= {}", "", indent, arg);
  }
  if constexpr (ivl::reflection::is_child_of(^^T, ^^std)) {
    return;
  } else {
    if constexpr (is_class_type(^^T)) {
      template for (constexpr auto base : ivl::reflection::bases(^^T)) {
        std::string_view access = "<unknown-access>";
        if (is_public(base)) access = "public";
        if (is_private(base)) access = "private";
        if (is_protected(base)) access = "protected";
        std::string_view virt = is_virtual(base) ? " virtual" : "";
        std::print("{: <{}}: {}{} ", "", indent, access, virt);
        describe(arg.[:base:], indent + 2);
      }
    }
    if constexpr (is_class_type(^^T) || is_union_type(^^T)) {
      template for (constexpr auto member : ivl::reflection::nsdms(^^T)) {
        std::string_view access = "<unknown-access>";
        if (is_public(member)) access = "public";
        if (is_private(member)) access = "private";
        if (is_protected(member)) access = "protected";
        std::string_view ident = "<unknown-identifier>";
        if constexpr (has_identifier(member)) ident = identifier_of(member);
        else if (is_bit_field(member)) ident = "<unnamed bit-field>";
        else ident = "<unnamed member>";
        std::print("{: <{}}  {} {}: ", "", indent, access, ident);
        if (is_class_type(^^T)) describe(arg.[:member:], indent + 2);
        else std::println("{}", ivl::reflection::display_string_of(type_of(member)));
      }
    }
  }
}

template <typename T>
void describe2(const T& arg, size_t indent = 0) {
  is_union_type(^^T) && ivl::panic("cant handle union type: {}", display_string_of(^^T));
  std::print("{}", ivl::reflection::display_string_of(^^T));
  if constexpr (std::formattable<T, char>) {
    if constexpr (debug_formattable<T>) std::println(" = {:?}", arg);
    else std::println(" = {}", arg);
  } else if constexpr (ivl::reflection::is_child_of(^^T, ^^std)) {
    ivl::panic("dunno what to do with {}", display_string_of(^^T));
  } else {
    static_assert(is_class_type(^^T), display_string_of(^^T));

    std::println();

    template for (constexpr auto base : ivl::reflection::bases(^^T)) {
      static_assert(is_public(base), display_string_of(base));
      static_assert(!is_virtual(base), display_string_of(base));
      std::print("{: <{}}: ", "", indent);
      describe2(arg.[:base:], indent + 2);
    }

    template for (constexpr auto member : ivl::reflection::nsdms(^^T)) {
      static_assert(is_public(member), display_string_of(member));
      static_assert(has_identifier(member), display_string_of(member));
      std::print("{: <{}}  {}: ", "", indent, identifier_of(member));
      describe2(arg.[:member:], indent + 2);
    }
  }
}

int ivl_main(cc_bundle& args) {
  args_bundle a2;
  std::string foo = "hello world";
  describe2(a2);
  return 0;
}
