#include <fstream>

// sudo apt install nlohmann-json3-dev
#include <nlohmann/json.hpp>

#include <ivl/logger>

using json = nlohmann::json;

template <typename T>
T& unmove(T&& t) {
  return static_cast<T&>(t);
}

// const json recipes = json::parse(std::ifstream{"recipes.json"});
const json recipes = json::parse(std::ifstream{"factorio-recipes.json"});
// const json all = json::parse(std::ifstream{"all.json"});

int main() {
  // std::cout << recipes.dump(2) << std::endl;
  // LOG(recipes[0]);
  for (auto& el : recipes)
    LOG(el["name"]);
}
