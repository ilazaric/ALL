#include <ivl/reflection/json>
#include <ivl/reflection/prettier_types>
#include <ivl/reflection/utility>
#include <ivl/utility>
#include "annotations"
#include "passthrough"
#include <ivl/format>
#include <ivl/format>

struct[[= ivl::cmdline_parsing::class_basic]] cc_bundle {
  bool bla;
  std::string truc;
  int x;
  float y;
  // const char* z; // json doesnt like this
};

int ivl_main(cc_bundle& args, ivl::cmdline_parsing::passthrough pass) {
  ivl::fmt::println("{:2}", ivl::to_json(args));
  return 0;
}
