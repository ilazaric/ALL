#pragma once

#include <ivl/exception>
#include <ivl/reflection/prettier_types>
#include <cassert>
#include <meta>
#include <optional>
#include <span>
#include <string_view>

namespace ivl {
struct text_annotation {
  const char* text;
  consteval text_annotation(auto&& data) : text(std::define_static_string(data)) {}
};

struct long_option : text_annotation {};

struct short_option : text_annotation {};

struct title : text_annotation {};

struct description : text_annotation {};

struct name : text_annotation {};

consteval std::vector<std::meta::info> wrap(std::vector<std::meta::info> v) {
  std::vector<std::meta::info> ret;
  for (auto i : v) ret.push_back(reflect_constant(i));
  return ret;
}

template <std::meta::info... Is>
inline auto foreach = [](auto&& callable) { ((callable.template operator()<Is>()), ...); };

consteval bool is_in_std(std::meta::info i) {
  for (; has_parent(i); i = parent_of(i))
    if (i == ^^std) return true;
  return false;
}

template <std::meta::info I>
void describe_descriptions(size_t indent) {
  [:substitute(^^foreach, wrap(annotations_of_with_type(I, ^^description))):]([&]<std::meta::info DI> {
    std::println("{: <{}}description:", "", indent * 2);
    std::println("{: <{}}- {}", "", indent * 2, extract<description>(DI).text);
  });
}

template <std::meta::info I>
void describe_member(size_t indent) {
  std::println("{: <{}}name: {}", "", indent * 2, identifier_of(I));
  std::println("{: <{}}type: {}", "", indent * 2, reflection::display_string_of(type_of(I)));
  describe_descriptions<I>(indent);
  std::println();
}

template <std::meta::info I>
void describe_bundle(size_t indent = 0) {
  std::println("{: <{}}name: {}", "", indent * 2, identifier_of(I));

  describe_descriptions<I>(indent);

  [:substitute(^^foreach, wrap(nonstatic_data_members_of(I, std::meta::access_context::unchecked()))):](
    [&]<std::meta::info mem>() { describe_member<mem>(indent + 1); }
  );
}

template <typename T>
T parse(std::span<const char*> args) {
  T state;

start:
  if (args.empty()) return state;

  auto arg = std::string_view(args[0]);
  args = args.subspan(1);

  if (!arg.starts_with('-')) throw ivl::base_exception("expected an option, received: `{}`", arg);

  if (!arg.starts_with("--")) throw ivl::base_exception("options should start with `--`, received: `{}`", arg);

  arg.remove_prefix(2);

  if (arg == "help") {
    describe_bundle<^^T>();
  }

  goto start;
}

[[= ivl::test]] void parse_test() {}
} // namespace ivl
