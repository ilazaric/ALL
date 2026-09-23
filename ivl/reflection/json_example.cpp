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
  std::cout << dump(ivl::to_json(t), 2) << std::endl;

  auto j = boost::json::object();
  j["foo"] = boost::json::value{};
  std::cout << j << std::endl;
}
