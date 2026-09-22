#define IVL_JSON_USE_NLOHMANN
#include "default"
#include <iostream>

int ivl_main(int indent) {
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
  std::cout << "dumping:\n" << v.dump(indent) << "\ndone\n";
  return 0;
}
