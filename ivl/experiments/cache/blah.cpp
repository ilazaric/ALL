#include <meta>
#include <print>

#include "define2"

// IVL add_compiler_flags("-Wno-subobject-linkage")

struct my_string_view;

consteval {
  define_aggregate_with_member_functions(
    (^^my_string_view),
    {
      data_member_spec((^^const char*), {.name = "ptr"}),
      data_member_spec((^^std::size_t), {.name = "len"}),
    },
    {
      {"size", [](auto&& self) { return self.len; }},
      {"empty", [](auto&& self) { return self.size() == 0; }},
    }
  );
}

static_assert(sizeof(my_string_view) == 16);

int main() {
  my_string_view sv;
  sv.ptr = "hello world";
  sv.len = 11;
  std::println("sv.size(): {}", sv.size());
  std::println("sv.empty(): {}", sv.empty());
}
