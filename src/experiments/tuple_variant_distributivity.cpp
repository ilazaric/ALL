#include <tuple>
#include <variant>

template<typename A, typename B, typename C>
using LHS = std::tuple<A, std::variant<B, C>>;

template<typename A, typename B, typename C>
using RHS = std::variant<std::tuple<A, B>, std::tuple<A, C>>;

