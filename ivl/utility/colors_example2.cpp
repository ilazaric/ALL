#include <ivl/utility/colors>
#include <ivl/format>

int main() {
  namespace term = ivl::terminal_graphical_rendition;
  ivl::fmt::println("... {} ...", term::colors::FG_RED("hello world {}", 42));
}
