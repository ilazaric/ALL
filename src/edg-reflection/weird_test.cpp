#include <experimental/meta>

struct S {
  static constexpr auto injection_scope = std::meta::nearest_token_queuing_context();
};

consteval {
  queue_injection(S::injection_scope, ^{constexpr int fn(){return 12;
}
});
}

static_assert(S {}.fn() == 12);
