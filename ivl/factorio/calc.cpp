#include <fstream>

// sudo apt install nlohmann-json3-dev
#include <ivl/json>

#include <ivl/logger>

using json = boost::json::value;

template <typename T>
T& unmove(T&& t) {
  return static_cast<T&>(t);
}

// const json recipes = json::parse(std::ifstream{"recipes.json"});
const json recipes = boost::json::parse(std::ifstream{"factorio-recipes.json"});
// const json all = json::parse(std::ifstream{"all.json"});

int main() {
  // std::cout << dump(recipes, 2) << std::endl;
  // LOG(recipes[0]);
  for (auto& el : recipes.as_array())
    LOG(el.as_object().at("name"));
}
