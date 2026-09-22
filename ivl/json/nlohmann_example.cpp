#define IVL_JSON_USE_NLOHMANN
#include "default"
#include <iostream>

int ivl_main(int64_t indent) {
  auto v = ivl::json::parse(R"json(
    {
      "null": null,
      "int": 123,
      "float": 1.23,
      "string": "string",
      "array": [
        1,
        2,
        3
      ],
      "object": {
        "a": 1,
        "b": 2
      }
    }
  )json");
  std::cout << "dumping:\n" << ivl::json::dump(v, (size_t)indent) << "\ndone\n";
  return 0;
}
