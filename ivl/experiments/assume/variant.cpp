#include <variant>
#include <string>

struct foo { std::string s; };
struct bar { std::string s; };

using var = std::variant<foo, bar>;

std::string& get_string_builtin_assume(var& v) {
    if (v.valueless_by_exception()) __builtin_unreachable();
    return std::visit([](auto& x) -> auto& { return x.s; }, v);
}

std::string& get_string_attr_assume(var& v) {
    [[assume(!v.valueless_by_exception())]];
    return std::visit([](auto& x) -> auto& { return x.s; }, v);
}
