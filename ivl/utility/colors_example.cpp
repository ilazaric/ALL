#include <ivl/utility/colors>
#include <ivl/format>

int main() {
  namespace term = ivl::terminal_graphical_rendition;
  for (int i = 0; i < 256; i += 5) {
    for (int j = 0; j < 256; j += 2) ivl::fmt::print("{} ", term::background_color{i, i, j});
    ivl::fmt::println("{}", term::background_reset{});
  }
  char foo[] = "hello\nworld";
  ivl::fmt::println("foo {:?} bar", "hello\nworld");
  ivl::fmt::println("foo {:?} bar", foo);
  auto clr = term::foreground_color{255, 0, 0};
  ivl::fmt::println("foo {} bar", clr("{:?}", "hello\nworld"));
  ivl::fmt::println("foo {:?} bar", clr("hello\nworld"));
}
