#include <ivl/reflection/argument_parsing>
#include <meta>

// description
// title
// short_option
// long_option

struct [[=ivl::title("Compilation Options"),
         =ivl::description("This is an example of a sub-bundle of command-line options. A sub-bundle ")]]
  compilation_options {
  [[=ivl::short_option("O"), =ivl::description("bundle arg")]] int optimization_level;
  [[=ivl::short_option("I"), =ivl::description("")]] std::vector<std::filesystem::path> include_directories;
  [[=ivl::long_option("std")]] std::string cxx_standard;
};

struct[[= ivl::description(
  "This is an example of command-line options. This description is for the top-level bundle. The description contains "
  "no newline characters. The printing detects the width of the terminal and formats this text appropriately. If "
  "stdout is not a terminal it defaults to 80."
)]] options {
  [[= ivl::description("first arg")]] int first;
  [[= ivl::description("second arg")]] std::string second;
  [[= ivl::description("third arg")]] bool third;

  compilation_options b;
};

// -Wl,--verbose

int main() {
  ivl::describe_bundle<^^options>();
  // auto args = ivl::parse<arguments>(std::span<const char*>(argv + 1, argv + 1 + argc));
}
