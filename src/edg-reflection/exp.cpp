#include <cassert>
#include <concepts>
#include <cstdint>
#include <experimental/meta>
#include <string_view>
#include <type_traits>

using namespace std::literals::string_view_literals;

// struct S {
//   [[=1]] int a;
// };

// consteval bool experiment(){
//   auto i = ^S;
//   auto els = nonstatic_data_members_of(i);
//   assert(els.size() == 1);
//   auto mem = els[0];
//   assert(has_annotation<int>(mem));
//   return true;
// }

// static_assert(experiment());

consteval void self_impl() {
  queue_injection(^{
    struct SelfDetailDummy;
    using Self = [:parent_of(^SelfDetailDummy):];
  });
}

consteval void self() {
  queue_injection(^{
    struct SelfDetailDummy;
    using Self = [:parent_of(^SelfDetailDummy):];
  });
  // queue_injection(^{
  //     consteval { if (!requires {typename SelfDetailDummy;}) self_impl(); }
  //   });
}

struct ABC {
  int   x;
  char  y;
  float z;

  // consteval { if constexpr (requires {typename Bla;}) std::cout << "hello"; }
  consteval { self(); }
  // consteval { self(); }
  // consteval { self(); }
};

// static_assert(![](auto){return requires{typename AAA;}}(0));

static_assert(std::is_same_v<ABC, ABC::Self>);

// consteval bool bla(){
//   auto znj = ^meaningless;
//   return true;
// }

// static_assert(bla());

consteval void magic_impl(std::meta::info base) {
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

    consteval void magic(std::string_view name, std::meta::info body) {
      queue_injection(^{
        struct \id(name, "_magic_detail"sv) { \tokens(body)};
      });

  queue_injection(^{
    struct \id(name)
        : \id(name, "_magic_detail"sv) {consteval {magic_impl(^\id(name, "_magic_detail"sv)); }
    };
    });
    }

    consteval {
      magic("XYZ", ^{
        int   x;
        char  y;
        float z;
      });
    }

    consteval bool test() {
      XYZ xyz;
      xyz.set_x(1);
      xyz.set_y(1);
      return xyz.get_x() == xyz.get_y();
    }

    static_assert(test());
