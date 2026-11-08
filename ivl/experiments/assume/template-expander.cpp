#include <ivl/format>
#include <ivl/linux/utility>
#include <ivl/logger>
#include <ivl/string/join>
#include <ivl/string/replace>
#include <ivl/string/split>
#include <ivl/string>
#include <algorithm>
#include <filesystem>
#include <ranges>

struct replacement {
  std::string key;
  std::vector<std::string> values;
};

// ./template-expander inherit-template.cpp inherit.{}.cpp

int ivl_main(const std::filesystem::path& input, const std::filesystem::path& output_form) {
  contract_assert(output_form.native().contains("{}"));
  auto contents = ivl::linux::read_file(input);
  auto lines = ivl::string::split(contents, "\n");
  auto curr = lines.begin();
  std::vector<replacement> replacements;
  while (curr != lines.end() && curr->starts_with("// {")) {
    contract_assert(curr->ends_with("}:"));
    auto key = curr->substr(3, curr->size() - 3 - 1);
    ++curr;
    std::vector<std::string> values;
    while (curr != lines.end() && curr->starts_with("// : ")) {
      values.push_back(curr->substr(5));
      ++curr;
    }
    ivl::fmt::println("replacement: {:?} -> {::?}", key, values);
    contract_assert(!values.empty());
    replacements.emplace_back(std::move(key), std::move(values));
  }
  LOG(replacements.size());
  contract_assert(!replacements.empty());

  auto remaining = ivl::string::join(std::ranges::subrange(curr, lines.end()), "\n");
  std::vector<size_t> choices;
  auto lambda = [&](this const auto& self, std::string_view current, size_t index) {
    if (index == replacements.size()) {
      std::filesystem::path output = ivl::string::replace(
        output_form.native(), "{}", ivl::string::join(choices | std::views::transform(ivl::to_string), "_")
      );
      std::string prefix;
      for (size_t i = 0; i < replacements.size(); ++i)
        prefix += std::format("// {}: {}: {}\n", replacements[i].key, choices[i], replacements[i].values[choices[i]]);
      ivl::fmt::println("generating {} ...", output);
      ivl::linux::write_file_slow(output, prefix + "\n" + current);
      return;
    }
    choices.emplace_back();
    auto& repl = replacements[index];
    for (size_t i = 0; i < repl.values.size(); ++i) {
      choices.back() = i;
      self(ivl::string::replace(current, repl.key, repl.values[i]), index + 1);
    }
    choices.pop_back();
  };
  lambda(remaining, 0);
  return 0;
}
