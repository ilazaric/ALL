#include <ivl/utility/colors>
#include <print>

int main() {
  namespace term = ivl::terminal_graphical_rendition;
  std::println("... {} ...", term::colors::FG_RED("hello world {}", 42));
}
