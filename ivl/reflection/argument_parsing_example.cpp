#include <ivl/reflection/argument_parsing>
#include <ivl/reflection/json>
#include <ivl/reflection/prettier_types>
#include <ivl/reflection/utility>
#include <ivl/utility>
#include <format>
#include <print>

struct cc_bundle {
  bool bla;
  std::string truc;
  int x;
  float y;
  // const char* z; // json doesnt like this
};

int ivl_main(cc_bundle& args, std::span<const char*> pass) {
  std::println("{}", ivl::to_json(args).dump(2));
  return 0;
}
