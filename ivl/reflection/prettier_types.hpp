#pragma once

#include <filesystem>
#include <meta>
#include <string>
#include <string_view>

namespace ivl::reflection {
consteval std::string_view display_string_of(std::meta::info r) {
  std::string ret(std::meta::display_string_of(r));

  auto replace = [&](std::string_view a, std::string_view b) {
    std::string_view before(ret);
    std::string after;
    while (true) {
      auto loc = before.find(a);
      if (loc == std::string_view::npos) break;
      after += before.substr(0, loc);
      after += b;
      before.remove_prefix(loc + a.size());
      // TODO: this cleans up some leftovers from "> >", maybe different approach warranted
      if (before.starts_with(' ')) before.remove_prefix(1);
    }
    ret = after + before;
  };

  replace(std::meta::display_string_of(dealias(^^std::string)), "std::string");
  replace(std::meta::display_string_of(dealias(^^std::string_view)), "std::string_view");
  replace(std::meta::display_string_of(dealias(^^std::filesystem::path)), "std::filesystem::path");

  return std::string_view(std::define_static_string(ret));
}

// struct diagnostics {

// };

// struct notes_container {
//   std::map<std::meta::info, std::string> notes;
// };

// consteval std::string describe_class(
//   std::meta::info type
//   // , const notes_container& notes
// ) {
//   if (!is_class_type(type) && !is_union_type(type))
//     throw std::meta::exception(std::format("argument must be a class type, got `{}`", display_string_of(type)));

//   std::string out;
//   std::format_to(
//     std::back_inserter(out), "// file: {}, line: {}\n", source_location(type).file_name(),
//     source_location(type).line()
//   );
//   std::format_to(std::back_inserter(out), "{} {} {{\n", is_class_type(type) ? "struct" : "union",
//   identifier_of(type));

//   enum access { a_public, a_protected, a_private, a_broken };
//   auto access_of = [](std::meta::info i) {
//     return is_public(i) ? a_public : is_protected(i) ? a_protected : is_private(i) ? a_private : a_broken;
//   };
//   auto access_str = [](access a) {
//     switch (a) {
//     case a_public:
//       return "public";
//     case a_protected:
//       return "protected";
//     case a_private:
//       return "private";
//     default:
//       return "<unknown-access-specifier>";
//     }
//   };

//   auto current_access = a_public;
//   size_t current_offset = 0;
//   for (auto member : nonstatic_data_members_of(type, std::meta::access_context::unchecked())) {
//     auto mem_access = access_of(member);
//     if (current_access != mem_access) {
//       std::format_to(std::back_inserter(out), "{}:\n", access_str(mem_access));
//       current_access = mem_access;
//     }
//     std::format_to(std::back_inserter(out), "  {} {};\n", display_string_of(type_of(member)), identifier_of(member));
//   }

//   std::format_to(std::back_inserter(out), "};\n");
//   return out;
// }
// }
} // namespace ivl::reflection
