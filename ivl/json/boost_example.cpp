#include "default"
#include <iostream>

int ivl_main(int64_t indent) {
  auto v = boost::json::parse(R"json(
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
  std::cout << "dumping:\n" << dump(v, (size_t)indent) << "\ndone\n";
  return 0;
}
