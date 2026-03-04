#include <ivl/utility/colors>
#include <print>

int main() {
  namespace term = ivl::terminal_graphical_rendition;
  for (int i = 0; i < 256; i += 5) {
    for (int j = 0; j < 256; j += 2) std::print("{} ", term::background_color{i, i, j});
    std::println("{}", term::background_reset{});
  }
  char foo[] = "hello\nworld";
  std::println("foo {:?} bar", "hello\nworld");
  std::println("foo {:?} bar", foo);
  auto clr = term::foreground_color{255, 0, 0};
  std::println("foo {} bar", clr("{:?}", "hello\nworld"));
}
