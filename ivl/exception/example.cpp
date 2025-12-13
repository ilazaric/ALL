#include <ivl/exception>
#include <print>

std::string_view strip_prefix_opt(std::string_view a, std::string_view b) {
  return a.starts_with(b) ? a.substr(b.size()) : a;
}

std::string foo(std::source_location l) {
  return std::format(
    "{}:'{}':{}", strip_prefix_opt(l.file_name(), "/home/ilazaric/repos/ALL/ivl/"), l.function_name(), l.line()
  );
}

void fn() {
  EXCEPTION_CONTEXT("foo");
  try {
    EXCEPTION_CONTEXT("bar");
    EXCEPTION_CONTEXT("baz");
    throw ivl::base_exception{"hello"};
  } catch (const ivl::base_exception& e) {
    throw;
    // for (auto&& el : e.added_context)
    //   std::println("{} {}", el.location, el.text);
    std::println("EXCEPTION FROM {}", foo(e.throw_location));
    for (auto&& el : e.added_context)
      std::println("ADDED CONTEXT FROM {}\n + {}", foo(el.location), el.text);
  }
}

int main() try {
  fn();
 } catch (const ivl::base_exception& e){
  e.dump();
 }
