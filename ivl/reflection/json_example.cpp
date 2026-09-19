#include <ivl/reflection/json>
#include <iostream>

struct S {
  std::string a;
  std::string b;
  int c;
  float d;
};

struct T {
  S x;
  S y;
  std::vector<S> z{{}, {}};
  std::map<std::string, S> w{{"foo", {}}, {"bar", {}}};
};

int main() {
  T t{};
  std::cout << ivl::to_json(t).dump(2) << std::endl;

  auto j = ivl::json::object();
  j["foo"] = ivl::json::value{};
  std::cout << j.get<ivl::json::value>() << std::endl;
}
