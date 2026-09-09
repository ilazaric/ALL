#include <print>
#include <source_location>
#include <string>
#include <string_view>

struct aggregate;

struct validator {
  static bool validate_name(std::string_view name) {
    for (auto c : name) {
      if (::isspace(c)) continue;
      if (::isalpha(c)) continue;
      std::print(
        stderr,
        "name validation failed!\n"
        "- name can only contain whitespace and alphabet characters\n"
        "- name: {:?}\n"
        "- bad character: {:?}\n"
        "| \n",
        name, c
      );
      return false;
    }
    return true;
  }

  static bool validate_address(std::string_view address) {
    for (auto c : address) {
      if (::isspace(c)) continue;
      if (::isalnum(c)) continue;
      std::print(
        stderr,
        "address validation failed!\n"
        "- address can only contain whitespace and alphanumeric characters\n"
        "- address: {:?}\n"
        "- bad character: {:?}\n"
        "| \n",
        address, c
      );
      return false;
    }
    return true;
  }

  static bool validate_age(int64_t age) {
    if (age < 0) {
      std::print(
        stderr,
        "age validation failed!\n"
        "- age must be a non-negative integer\n"
        "- age: {}\n"
        "| \n",
        age
      );
      return false;
    }
    return true;
  }

  template<typename T = aggregate>
  validator(std::source_location loc = std::source_location::current()) {
    auto& agg = static_cast<T&>(*this);
    bool seen_errors = false;
    seen_errors |= !validate_name(agg.name);
    seen_errors |= !validate_address(agg.address);
    seen_errors |= !validate_age(agg.age);
    if (!seen_errors) return;
    std::print(
      stderr,
      "seen errors during aggregate validation, reporting code location:\n"
      "- file name: {}\n"
      "- function name: {}\n"
      "- row: {}\n"
      "- column: {}\n"
      "\n",
      loc.file_name(), loc.function_name(), loc.line(), loc.column()
    );
  }
};

struct aggregate_base {
  std::string name;
  std::string address;
  int64_t age;
};

struct aggregate : aggregate_base, validator {};

int main() {
  aggregate a{
    .name = "john doe",
    .address = "paris",
    .age = 42,
  };
  aggregate b{
    .name = "<NULL>",
    .address = "<NULL>",
    .age = -9999,
  };
}
