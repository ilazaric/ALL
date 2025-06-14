#pragma once

#include <experimental/meta>
#include <string_view>

namespace ivl::refl {

  using namespace std::literals::string_view_literals;

  consteval void getters_setters_impl(std::meta::info base) {
    for (auto el : nonstatic_data_members_of(base)) {
      auto name = identifier_of(el);
    queue_injection(^{
        constexpr const auto& \id("get_"sv, name)() const {return \id(name);
    }
  });
    queue_injection(^{
        constexpr void \id("set_"sv, name)(auto&& arg) {\id(name) = std::forward<decltype(arg)>(arg);
}
});
}
}

consteval void getters_setters(std::string_view name, std::meta::info body) {
  queue_injection(^{
    struct \id(name, "_getters_setters_detail"sv) { \tokens(body)};
  });

  queue_injection(^{
    struct \id(name)
        : \id(name, "_getters_setters_detail"sv) {
          consteval {getters_setters_impl(^\id(name, "_getters_setters_detail"sv)); }
};
});
}

} // namespace ivl::refl
